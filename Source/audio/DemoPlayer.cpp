#include "audio/DemoPlayer.h"
#include <algorithm>
#include <cmath>

namespace lumen::audio
{

void DemoPlayer::prepare(double sampleRateHertz, int maximumBlockSizeSamples)
{
    currentSampleRate = sampleRateHertz > 0.0 ? sampleRateHertz : 44100.0;
    maxBlock = juce::jmax(1, maximumBlockSizeSamples);
    prepared.store(true, std::memory_order_relaxed);
    refreshClipList();
    ensureAudioLoaded();
}

void DemoPlayer::reset()
{
    readIndex.store(0, std::memory_order_relaxed);
}

void DemoPlayer::setClipDirectory(const juce::File& directory)
{
    clipDirectory = directory;
    refreshClipList();
    ensureAudioLoaded();
}

juce::Array<DemoClipInfo> DemoPlayer::getClips() const
{
    const juce::ScopedLock lock(clipLock);
    return clips;
}

juce::String DemoPlayer::getActiveClipId() const
{
    const juce::ScopedLock lock(clipLock);
    return activeClipId;
}

juce::String DemoPlayer::getActiveClipName() const
{
    const juce::ScopedLock lock(clipLock);
    return activeClipName;
}

bool DemoPlayer::hasAudio() const noexcept
{
    return hasSamples.load(std::memory_order_relaxed);
}

void DemoPlayer::ensureAudioLoaded()
{
    {
        const juce::ScopedLock lock(clipLock);
        if (hasSamples.load(std::memory_order_relaxed) && !monoSamples.empty())
        {
            if (activeClipId.isNotEmpty())
                return;
        }
    }

    juce::String idToLoad;
    {
        const juce::ScopedLock lock(clipLock);
        if (activeClipId.isNotEmpty())
            idToLoad = activeClipId;
        else if (!clips.isEmpty())
            idToLoad = clips.getReference(0).id;
    }

    if (idToLoad.isNotEmpty())
        selectClip(idToLoad);
    else
        ensureFallbackClip();
}

bool DemoPlayer::selectClip(const juce::String& clipId)
{
    juce::File fileToLoad;
    juce::String name;
    const auto wanted = clipId.trim();

    {
        const juce::ScopedLock lock(clipLock);
        if (wanted.isEmpty() && !clips.isEmpty())
        {
            fileToLoad = clips.getReference(0).file;
            name = clips.getReference(0).name;
        }
        else
        {
            for (const auto& clip : clips)
            {
                if (clip.id == wanted)
                {
                    fileToLoad = clip.file;
                    name = clip.name;
                    break;
                }
            }
        }
    }

    if (wanted == "fallback" || fileToLoad.getFullPathName().isEmpty() || !fileToLoad.existsAsFile())
    {
        if (wanted == "fallback" || clips.isEmpty())
        {
            ensureFallbackClip();
            return hasSamples.load(std::memory_order_relaxed);
        }

        // Unknown id with clips present — load first clip.
        {
            const juce::ScopedLock lock(clipLock);
            if (!clips.isEmpty())
            {
                fileToLoad = clips.getReference(0).file;
                name = clips.getReference(0).name;
            }
        }
        if (!fileToLoad.existsAsFile())
        {
            ensureFallbackClip();
            return hasSamples.load(std::memory_order_relaxed);
        }
    }

    if (!loadClipFile(fileToLoad))
    {
        ensureFallbackClip();
        return hasSamples.load(std::memory_order_relaxed);
    }

    {
        const juce::ScopedLock lock(clipLock);
        activeClipId = fileToLoad.existsAsFile()
                           ? fileToLoad.getFileNameWithoutExtension()
                           : (wanted.isNotEmpty() ? wanted : "fallback");
        activeClipName = name.isNotEmpty() ? name : activeClipId.replaceCharacter('_', ' ');
    }

    readIndex.store(0, std::memory_order_relaxed);
    return true;
}

void DemoPlayer::start()
{
    // Allow demo before the host has opened the audio device.
    if (!prepared.load(std::memory_order_relaxed))
        prepared.store(true, std::memory_order_relaxed);

    ensureAudioLoaded();

    if (!hasSamples.load(std::memory_order_relaxed))
        ensureFallbackClip();

    if (!hasSamples.load(std::memory_order_relaxed))
        return;

    readIndex.store(0, std::memory_order_relaxed);
    playing.store(true, std::memory_order_release);
}

void DemoPlayer::stop()
{
    playing.store(false, std::memory_order_release);
    readIndex.store(0, std::memory_order_relaxed);
}

bool DemoPlayer::toggle()
{
    if (isPlaying())
    {
        stop();
        return false;
    }

    start();
    return isPlaying();
}

bool DemoPlayer::isPlaying() const noexcept
{
    return playing.load(std::memory_order_relaxed);
}

void DemoPlayer::processReplaceInput(juce::AudioBuffer<float>& buffer)
{
    if (!prepared.load(std::memory_order_relaxed)
        || !playing.load(std::memory_order_acquire)
        || !hasSamples.load(std::memory_order_relaxed))
        return;

    const int sampleCount = buffer.getNumSamples();
    const int channelCount = buffer.getNumChannels();
    if (sampleCount <= 0 || channelCount <= 0)
        return;

    const float* data = nullptr;
    int total = 0;

    {
        const juce::ScopedLock lock(clipLock);
        if (monoSamples.empty())
            return;
        data = monoSamples.data();
        total = static_cast<int>(monoSamples.size());
    }

    if (data == nullptr || total <= 0)
        return;

    int index = readIndex.load(std::memory_order_relaxed);

    for (int i = 0; i < sampleCount; ++i)
    {
        if (index >= total)
            index = 0;

        const float sample = data[index++];
        for (int ch = 0; ch < channelCount; ++ch)
            buffer.setSample(ch, i, sample);
    }

    readIndex.store(index, std::memory_order_relaxed);
}

void DemoPlayer::refreshClipList()
{
    juce::Array<DemoClipInfo> next;

    if (clipDirectory.isDirectory())
    {
        const auto files = clipDirectory.findChildFiles(
            juce::File::findFiles,
            false,
            "*.wav");

        for (const auto& file : files)
        {
            DemoClipInfo info;
            info.file = file;
            info.id = file.getFileNameWithoutExtension();
            info.name = info.id.replaceCharacter('_', ' ');
            next.add(std::move(info));
        }
    }

    std::sort(next.begin(), next.end(), [](const DemoClipInfo& a, const DemoClipInfo& b) {
        return a.name.compareIgnoreCase(b.name) < 0;
    });

    // Always expose a guaranteed built-in entry.
    {
        DemoClipInfo fallback;
        fallback.id = "fallback";
        fallback.name = "Built-in DI";
        next.add(fallback);
    }

    {
        const juce::ScopedLock lock(clipLock);
        clips = std::move(next);

        bool found = false;
        if (activeClipId.isNotEmpty())
        {
            for (const auto& clip : clips)
            {
                if (clip.id == activeClipId)
                {
                    found = true;
                    break;
                }
            }
        }

        if (!found)
        {
            // Prefer a real WAV over fallback when available.
            for (const auto& clip : clips)
            {
                if (clip.id != "fallback" && clip.file.existsAsFile())
                {
                    activeClipId = clip.id;
                    activeClipName = clip.name;
                    found = true;
                    break;
                }
            }
            if (!found && !clips.isEmpty())
            {
                activeClipId = clips.getReference(0).id;
                activeClipName = clips.getReference(0).name;
            }
        }
    }
}

bool DemoPlayer::loadClipFile(const juce::File& file)
{
    juce::AudioFormatManager formats;
    formats.registerBasicFormats();
    std::unique_ptr<juce::AudioFormatReader> reader(formats.createReaderFor(file));
    if (reader == nullptr || reader->lengthInSamples <= 0)
        return false;

    const auto sourceLength = static_cast<int>(
        juce::jmin(reader->lengthInSamples, static_cast<juce::int64>(srToSamples(30.0))));
    if (sourceLength <= 0)
        return false;

    juce::AudioBuffer<float> temp(static_cast<int>(juce::jmax(1u, reader->numChannels)), sourceLength);
    reader->read(&temp, 0, sourceLength, 0, true, true);

    std::vector<float> mono(static_cast<size_t>(sourceLength), 0.0f);
    const int channels = temp.getNumChannels();
    for (int i = 0; i < sourceLength; ++i)
    {
        float sum = 0.0f;
        for (int ch = 0; ch < channels; ++ch)
            sum += temp.getSample(ch, i);
        mono[static_cast<size_t>(i)] = sum / static_cast<float>(juce::jmax(1, channels));
    }

    if (std::abs(reader->sampleRate - currentSampleRate) > 1.0 && reader->sampleRate > 0.0)
    {
        const double ratio = currentSampleRate / reader->sampleRate;
        const int outLength = juce::jmax(1, static_cast<int>(std::llround(sourceLength * ratio)));
        std::vector<float> resampled(static_cast<size_t>(outLength), 0.0f);

        for (int i = 0; i < outLength; ++i)
        {
            const double srcPos = static_cast<double>(i) / ratio;
            const int i0 = static_cast<int>(srcPos);
            const int i1 = juce::jmin(sourceLength - 1, i0 + 1);
            const float frac = static_cast<float>(srcPos - static_cast<double>(i0));
            const float a = mono[static_cast<size_t>(juce::jlimit(0, sourceLength - 1, i0))];
            const float b = mono[static_cast<size_t>(i1)];
            resampled[static_cast<size_t>(i)] = a + (b - a) * frac;
        }

        mono = std::move(resampled);
    }

    {
        const juce::ScopedLock lock(clipLock);
        monoSamples = std::move(mono);
        hasSamples.store(!monoSamples.empty(), std::memory_order_release);
    }

    return hasSamples.load(std::memory_order_relaxed);
}

void DemoPlayer::ensureFallbackClip()
{
    const double rate = currentSampleRate > 0.0 ? currentSampleRate : 44100.0;
    const int length = juce::jmax(1, static_cast<int>(rate * 4.0));
    std::vector<float> mono(static_cast<size_t>(length), 0.0f);

    const double notes[] = { 164.81, 246.94, 329.63, 392.00, 329.63, 246.94 };
    const int noteCount = static_cast<int>(sizeof(notes) / sizeof(notes[0]));
    const double noteDur = 0.55;

    for (int n = 0; n < noteCount; ++n)
    {
        const int start = static_cast<int>(n * noteDur * rate);
        const int end = juce::jmin(length, start + static_cast<int>(0.8 * rate));
        const double freq = notes[n];

        for (int i = start; i < end; ++i)
        {
            const double t = static_cast<double>(i - start) / rate;
            const double env = t < 0.01 ? (t / 0.01) : std::exp(-(t - 0.01) / 0.35);
            double saw = 0.0;
            for (int h = 1; h <= 6; ++h)
                saw += std::sin(2.0 * juce::MathConstants<double>::pi * freq * t * h) / h;
            mono[static_cast<size_t>(i)] += static_cast<float>(0.22 * env * saw);
        }
    }

    {
        const juce::ScopedLock lock(clipLock);
        monoSamples = std::move(mono);
        hasSamples.store(!monoSamples.empty(), std::memory_order_release);
        activeClipId = "fallback";
        activeClipName = "Built-in DI";

        bool hasFallback = false;
        for (const auto& clip : clips)
        {
            if (clip.id == "fallback")
            {
                hasFallback = true;
                break;
            }
        }
        if (!hasFallback)
        {
            DemoClipInfo info;
            info.id = activeClipId;
            info.name = activeClipName;
            clips.add(info);
        }
    }
}

int DemoPlayer::srToSamples(double seconds) const
{
    const double rate = currentSampleRate > 0.0 ? currentSampleRate : 44100.0;
    return juce::jmax(1, static_cast<int>(std::llround(seconds * rate)));
}

} // namespace lumen::audio
