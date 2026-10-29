#pragma once

#include <JuceHeader.h>
#include <atomic>
#include <vector>

namespace lumen::audio
{

struct DemoClipInfo
{
    juce::String id;
    juce::String name;
    juce::File file;
};

class DemoPlayer
{
public:
    void prepare(double sampleRateHertz, int maximumBlockSizeSamples);
    void reset();

    void setClipDirectory(const juce::File& directory);
    juce::Array<DemoClipInfo> getClips() const;
    juce::String getActiveClipId() const;
    juce::String getActiveClipName() const;
    bool hasAudio() const noexcept;

    bool selectClip(const juce::String& clipId);
    void start();
    void stop();
    bool isPlaying() const noexcept;
    bool toggle();

    void processReplaceInput(juce::AudioBuffer<float>& buffer);

private:
    bool loadClipFile(const juce::File& file);
    void refreshClipList();
    void ensureFallbackClip();
    void ensureAudioLoaded();
    int srToSamples(double seconds) const;

    double currentSampleRate = 44100.0;
    int maxBlock = 0;

    juce::File clipDirectory;
    juce::Array<DemoClipInfo> clips;
    juce::String activeClipId;
    juce::String activeClipName;

    // Audio thread reads this under clipLock for pointer/size snapshot only.
    std::vector<float> monoSamples;
    std::atomic<int> readIndex{0};
    std::atomic<bool> playing{false};
    std::atomic<bool> prepared{false};
    std::atomic<bool> hasSamples{false};
    mutable juce::CriticalSection clipLock;
};

} // namespace lumen::audio
