#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace lumen::practice
{

enum class PracticeEventKind : uint8_t
{
    // value = beat index within the bar (0 is the accented downbeat).
    beat = 0,
    // value = attack strength (fast/slow envelope ratio).
    onset = 1,
    // value = frequency in Hz (0 when unvoiced), confidence = autocorrelation confidence.
    pitch = 2,
};

// All events share one clock: the absolute sample index since the pipeline was prepared.
// That is what lets the UI line a pick up against a metronome click.
struct PracticeEvent
{
    PracticeEventKind kind = PracticeEventKind::beat;
    int64_t sampleIndex = 0;
    float value = 0.0f;
    float confidence = 0.0f;
};

// Single-producer (audio thread) / single-consumer (message thread) ring buffer.
// Kept JUCE-free and allocation-free so it is safe on the audio thread and testable in isolation.
// When full, new events are dropped rather than blocking the audio thread; droppedCount()
// lets the UI warn that a capture is incomplete.
template <size_t Capacity>
class PracticeEventQueue
{
    static_assert(Capacity >= 2 && (Capacity & (Capacity - 1)) == 0,
                  "Capacity must be a power of two so index wrapping is a cheap mask");

public:
    bool push(const PracticeEvent& event) noexcept
    {
        const size_t write = writeIndex.load(std::memory_order_relaxed);
        const size_t read = readIndex.load(std::memory_order_acquire);
        if (write - read >= Capacity)
        {
            dropped.fetch_add(1, std::memory_order_relaxed);
            return false;
        }

        events[write & indexMask] = event;
        writeIndex.store(write + 1, std::memory_order_release);
        return true;
    }

    // Moves up to maxEvents into destination; returns how many were moved.
    size_t pop(PracticeEvent* destination, size_t maxEvents) noexcept
    {
        const size_t read = readIndex.load(std::memory_order_relaxed);
        const size_t write = writeIndex.load(std::memory_order_acquire);
        const size_t available = write - read;
        const size_t count = available < maxEvents ? available : maxEvents;

        for (size_t offset = 0; offset < count; ++offset)
            destination[offset] = events[(read + offset) & indexMask];

        readIndex.store(read + count, std::memory_order_release);
        return count;
    }

    // Consumer-side only: discards everything currently queued.
    void clear() noexcept
    {
        readIndex.store(writeIndex.load(std::memory_order_acquire), std::memory_order_release);
        dropped.store(0, std::memory_order_relaxed);
    }

    uint32_t droppedCount() const noexcept { return dropped.load(std::memory_order_relaxed); }

private:
    static constexpr size_t indexMask = Capacity - 1;

    std::array<PracticeEvent, Capacity> events{};
    // Monotonic counters (never wrapped); unsigned overflow keeps write - read correct.
    std::atomic<size_t> writeIndex{0};
    std::atomic<size_t> readIndex{0};
    std::atomic<uint32_t> dropped{0};
};

} // namespace lumen::practice
