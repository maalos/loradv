#include <config.h>

Preferences preferences;

std::map<const char *, float> IRAM_ATTR settingsCache;

std::map<const char *, float> defaultSettings = {
    // max 16 chars
    {"Codec2 mode    ", CODEC2_MODE_1200},
    {"C2 LPCPF enable", CODEC2_LPC_PF_ENABLE},
    {"C2 LPCPF bassb.", CODEC2_LPC_PF_BASSBOOST},
    {"C2 LPCPF beta  ", CODEC2_LPC_PF_BETA},
    {"C2 LPCPF gamma ", CODEC2_LPC_PF_GAMMA},
    {"Frequency      ", LORA_RADIO_FREQ},
    {"Bandwidth      ", LORA_RADIO_BW},
    {"Spreading fact.", LORA_RADIO_SF},
    {"Coding rate    ", LORA_RADIO_CR},
    {"Sync word      ", LORA_RADIO_SYNC},
    {"Output power   ", LORA_RADIO_PWR},
    {"Preamble length", LORA_RADIO_PL},
    {"CRC length     ", LORA_RADIO_CRC},
};

std::map<const char *, const char *> abbreviations = {
    {"C2MODE", "Codec2 mode    "},
    {"C2LPCEN", "C2 LPCPF enable"},
    {"C2LPCBB", "C2 LPCPF bassb."},
    {"C2LPCBE", "C2 LPCPF beta  "},
    {"C2LPCGA", "C2 LPCPF gamma "},
    {"FQ", "Frequency      "},
    {"BW", "Bandwidth      "},
    {"SF", "Spreading fact."},
    {"CR", "Coding rate    "},
    {"SW", "Sync word      "},
    {"TXP", "Output power   "},
    {"PL", "Preamble length"},
    {"CRC", "CRC length     "},
};

void set_default_settings()
{
    LOG_INFO("Setting default settings");
    for (const auto &pair : defaultSettings)
    {
        preferences.putFloat(pair.first, pair.second);
    }
}

void setup_preferences()
{
    LOG_INFO("Setting up preferences");
    preferences.begin("loradv", false);

    // do we need defaults?
    if (preferences.getFloat("Frequency       ", 0.0) == 0.0)
        set_default_settings();
    
    // load into cache
    for (const auto &pair : defaultSettings)
    {
        float value = preferences.getFloat(pair.first, -1); // -1 if not found
        if (value != -1)
        {
            settingsCache[pair.first] = value; // set default
        }
        else
        {
            settingsCache[pair.first] = pair.second;
            preferences.putFloat(pair.first, pair.second);
        }
    }
}

void setSetting(const char *key, float value)
{
    auto it = abbreviations.find(key);
    if (it != abbreviations.end())
    {
        key = it->second;
    }

    settingsCache[key] = value;

    preferences.putFloat(key, value);
}

float getSetting(const char *key)
{
    auto it = abbreviations.find(key);
    if (it != abbreviations.end())
    {
        key = it->second;
    }

    auto cacheIt = settingsCache.find(key);
    if (cacheIt != settingsCache.end())
    {
        return cacheIt->second; // return from cache
    }

    return preferences.getFloat(key); // if not in cache return from flash
}