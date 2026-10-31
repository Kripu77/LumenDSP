#include "practice/PracticeLogStore.h"
#include "presets/UserSettings.h"

namespace lumen::practice
{

PracticeLogStore::PracticeLogStore(juce::File logFileToUse)
    : logFile(std::move(logFileToUse))
{
}

juce::File PracticeLogStore::getDefaultLogFile()
{
    return lumen::presets::UserSettings::getSettingsDirectory().getChildFile("practice-log.json");
}

juce::var PracticeLogStore::load() const
{
    if (!logFile.existsAsFile() || logFile.getSize() > maximumLogBytes)
        return juce::var(new juce::DynamicObject());

    const auto parsed = juce::JSON::parse(logFile.loadFileAsString());
    return parsed.isObject() ? parsed : juce::var(new juce::DynamicObject());
}

bool PracticeLogStore::save(const juce::var& log, juce::String* errorOut) const
{
    auto fail = [errorOut](const juce::String& reason) {
        if (errorOut != nullptr)
            *errorOut = reason;
        return false;
    };

    if (!log.isObject())
        return fail("Practice log must be a JSON object");

    const auto json = juce::JSON::toString(log, true);
    if (static_cast<int64_t>(json.getNumBytesAsUTF8()) > maximumLogBytes)
        return fail("Practice log is too large to save");

    if (const auto created = logFile.getParentDirectory().createDirectory(); created.failed())
        return fail("Could not create settings folder: " + created.getErrorMessage());

    // replaceWithText writes to a temporary file and swaps it in, so a crash mid-write
    // cannot leave a half-written log behind.
    if (!logFile.replaceWithText(json))
        return fail("Could not write practice log");

    return true;
}

} // namespace lumen::practice
