#pragma once

#include <JuceHeader.h>

namespace lumen::presets
{

struct CalibrationProfile
{
    juce::String id;
    juce::String name;
    float gainDb = 0.0f;
};

class UserSettings
{
public:
    UserSettings();

    void load();
    void save() const;

    juce::Array<CalibrationProfile> getCalibrationProfiles() const;
    juce::String getActiveCalibrationId() const;
    float getActiveCalibrationGainDb() const;
    juce::String getActiveCalibrationName() const;

    bool setActiveCalibration(const juce::String& profileId);
    bool upsertCalibrationProfile(const juce::String& name, float gainDb, juce::String* idOut = nullptr);
    bool deleteCalibrationProfile(const juce::String& profileId);
    void setActiveCalibrationGainDb(float gainDb);

    static juce::File getSettingsDirectory();
    static juce::File getSettingsFile();

private:
    void ensureDefaults();
    static juce::String makeId();

    juce::Array<CalibrationProfile> calibrationProfiles;
    juce::String activeCalibrationId;
};

} // namespace lumen::presets
