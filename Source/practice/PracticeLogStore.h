#pragma once

#include <JuceHeader.h>

namespace lumen::practice
{

// Persists the practice log (sessions, scores, targets) as one JSON document.
//
// The schema is owned by the web UI (Resources/web/practice/practiceLog.js); this class only
// validates that it is a JSON object of sane size and writes it atomically. Keeping storage on
// the C++ side matters because WebView storage is unreliable across platforms (Windows uses a
// temp-folder profile, and the custom juce:// origin is not guaranteed to persist).
class PracticeLogStore
{
public:
    // A year of daily sessions is well under 1 MB; anything larger is almost certainly a bug.
    static constexpr int64_t maximumLogBytes = 4 * 1024 * 1024;

    explicit PracticeLogStore(juce::File logFileToUse = getDefaultLogFile());

    // Returns the stored object, or an empty object when the file is missing or unreadable.
    juce::var load() const;

    // Returns false (and leaves the file untouched) when the payload is not an object,
    // is too large, or the write fails.
    bool save(const juce::var& log, juce::String* errorOut = nullptr) const;

    static juce::File getDefaultLogFile();

private:
    juce::File logFile;
};

} // namespace lumen::practice
