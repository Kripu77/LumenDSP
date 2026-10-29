#include "presets/UserSettings.h"

namespace lumen::presets
{

UserSettings::UserSettings()
{
    load();
}

juce::File UserSettings::getSettingsDirectory()
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("LumenDSP")
        .getChildFile("settings");
}

juce::File UserSettings::getSettingsFile()
{
    return getSettingsDirectory().getChildFile("user-settings.json");
}

void UserSettings::ensureDefaults()
{
    if (calibrationProfiles.isEmpty())
    {
        CalibrationProfile flat;
        flat.id = "default";
        flat.name = "Flat";
        flat.gainDb = 0.0f;
        calibrationProfiles.add(flat);

        CalibrationProfile hot;
        hot.id = "hot-interface";
        hot.name = "Hot interface";
        hot.gainDb = -12.0f;
        calibrationProfiles.add(hot);

        CalibrationProfile quiet;
        quiet.id = "quiet-pickup";
        quiet.name = "Quiet pickup";
        quiet.gainDb = 6.0f;
        calibrationProfiles.add(quiet);
    }

    if (activeCalibrationId.isEmpty())
        activeCalibrationId = calibrationProfiles.getReference(0).id;
}

void UserSettings::load()
{
    calibrationProfiles.clear();
    activeCalibrationId.clear();

    const auto file = getSettingsFile();
    if (file.existsAsFile())
    {
        const auto parsed = juce::JSON::parse(file.loadFileAsString());
        if (parsed.isObject())
        {
            activeCalibrationId = parsed.getProperty("activeCalibrationId", {}).toString();

            if (const auto* profiles = parsed.getProperty("calibrationProfiles", {}).getArray())
            {
                for (const auto& item : *profiles)
                {
                    if (!item.isObject())
                        continue;

                    CalibrationProfile profile;
                    profile.id = item.getProperty("id", {}).toString();
                    profile.name = item.getProperty("name", {}).toString();
                    profile.gainDb = static_cast<float>(static_cast<double>(item.getProperty("gainDb", 0.0)));
                    if (profile.id.isEmpty())
                        profile.id = makeId();
                    if (profile.name.isEmpty())
                        profile.name = "Profile";
                    profile.gainDb = juce::jlimit(-24.0f, 24.0f, profile.gainDb);
                    calibrationProfiles.add(profile);
                }
            }
        }
    }

    ensureDefaults();

    bool activeFound = false;
    for (const auto& profile : calibrationProfiles)
    {
        if (profile.id == activeCalibrationId)
        {
            activeFound = true;
            break;
        }
    }
    if (!activeFound)
        activeCalibrationId = calibrationProfiles.getReference(0).id;
}

void UserSettings::save() const
{
    auto* root = new juce::DynamicObject();
    root->setProperty("schemaVersion", 1);
    root->setProperty("activeCalibrationId", activeCalibrationId);

    juce::Array<juce::var> profiles;
    for (const auto& profile : calibrationProfiles)
    {
        auto* item = new juce::DynamicObject();
        item->setProperty("id", profile.id);
        item->setProperty("name", profile.name);
        item->setProperty("gainDb", profile.gainDb);
        profiles.add(juce::var(item));
    }
    root->setProperty("calibrationProfiles", profiles);

    getSettingsDirectory().createDirectory();
    getSettingsFile().replaceWithText(juce::JSON::toString(juce::var(root), true));
}

juce::Array<CalibrationProfile> UserSettings::getCalibrationProfiles() const
{
    return calibrationProfiles;
}

juce::String UserSettings::getActiveCalibrationId() const
{
    return activeCalibrationId;
}

float UserSettings::getActiveCalibrationGainDb() const
{
    for (const auto& profile : calibrationProfiles)
        if (profile.id == activeCalibrationId)
            return profile.gainDb;
    return 0.0f;
}

juce::String UserSettings::getActiveCalibrationName() const
{
    for (const auto& profile : calibrationProfiles)
        if (profile.id == activeCalibrationId)
            return profile.name;
    return {};
}

bool UserSettings::setActiveCalibration(const juce::String& profileId)
{
    for (const auto& profile : calibrationProfiles)
    {
        if (profile.id == profileId)
        {
            activeCalibrationId = profileId;
            save();
            return true;
        }
    }
    return false;
}

bool UserSettings::upsertCalibrationProfile(
    const juce::String& name,
    float gainDb,
    juce::String* idOut)
{
    const auto trimmed = name.trim();
    if (trimmed.isEmpty())
        return false;

    const float clamped = juce::jlimit(-24.0f, 24.0f, gainDb);

    for (auto& profile : calibrationProfiles)
    {
        if (profile.name.equalsIgnoreCase(trimmed))
        {
            profile.gainDb = clamped;
            activeCalibrationId = profile.id;
            if (idOut != nullptr)
                *idOut = profile.id;
            save();
            return true;
        }
    }

    CalibrationProfile profile;
    profile.id = makeId();
    profile.name = trimmed;
    profile.gainDb = clamped;
    calibrationProfiles.add(profile);
    activeCalibrationId = profile.id;
    if (idOut != nullptr)
        *idOut = profile.id;
    save();
    return true;
}

bool UserSettings::deleteCalibrationProfile(const juce::String& profileId)
{
    if (calibrationProfiles.size() <= 1)
        return false;

    for (int i = 0; i < calibrationProfiles.size(); ++i)
    {
        if (calibrationProfiles.getReference(i).id == profileId)
        {
            calibrationProfiles.remove(i);
            if (activeCalibrationId == profileId)
                activeCalibrationId = calibrationProfiles.getReference(0).id;
            save();
            return true;
        }
    }
    return false;
}

void UserSettings::setActiveCalibrationGainDb(float gainDb)
{
    const float clamped = juce::jlimit(-24.0f, 24.0f, gainDb);
    for (auto& profile : calibrationProfiles)
    {
        if (profile.id == activeCalibrationId)
        {
            profile.gainDb = clamped;
            save();
            return;
        }
    }
}

juce::String UserSettings::makeId()
{
    return juce::Uuid().toDashedString();
}

} // namespace lumen::presets
