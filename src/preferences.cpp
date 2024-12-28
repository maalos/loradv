#include <config.h>

const Setting defaultSettings[] = {
    {"C2MODE",  "Codec2 mode    ", CODEC2_MODE_1200},
    {"C2LPCEN", "C2 LPCPF enable", CODEC2_LPC_PF_ENABLE},
    {"C2LPCBB", "C2 LPCPF bassb.", CODEC2_LPC_PF_BASSBOOST},
    {"C2LPCBE", "C2 LPCPF beta  ", CODEC2_LPC_PF_BETA},
    {"C2LPCGA", "C2 LPCPF gamma ", CODEC2_LPC_PF_GAMMA},
    {"FQ",      "Frequency      ", LORA_RADIO_FREQ},
    {"BW",      "Bandwidth      ", LORA_RADIO_BW},
    {"SF",      "Spreading fact.", LORA_RADIO_SF},
    {"CR",      "Coding rate    ", LORA_RADIO_CR},
    {"SW",      "Sync word      ", LORA_RADIO_SYNC},
    {"TXP",     "Output power   ", LORA_RADIO_PWR},
    {"PL",      "Preamble length", LORA_RADIO_PL},
    {"CRC",     "CRC length     ", LORA_RADIO_CRC},
};

const char *resolveToAbbreviation(const char *key) {
    for (const auto &setting : defaultSettings) {
        if (strcmp(key, setting.fullKey) == 0) {
            return setting.abbreviation;
        }
    }
    return key;
}

const char *resolveToFullKey(const char *abbreviation) {
    for (const auto &setting : defaultSettings) {
        if (strcmp(abbreviation, setting.abbreviation) == 0) {
            return setting.fullKey;
        }
    }
    return abbreviation;
}

String getSettingFilePath(const char *abbreviation) {
    return String(SETTINGS_DIR) + "/" + abbreviation;
}

void setDefaultSettings() {
    Serial.println(F("Setting default settings"));

    if (!LittleFS.exists(SETTINGS_DIR)) {
        LittleFS.mkdir(SETTINGS_DIR);
    }

    for (const auto &setting : defaultSettings) {
        String filePath = getSettingFilePath(setting.abbreviation);
        fs::File file = LittleFS.open(filePath, "w");
        if (file) {
            file.printf("%.6f", setting.defaultValue);
            file.close();
        }
    }
}

void setupPreferences() {
    Serial.println(F("Setting up preferences"));
    if (!LittleFS.begin(true)) {
        Serial.println(F("LittleFS mount failed"));
        return;
    }

    // defaults needed?
    if (!LittleFS.exists(getSettingFilePath("FQ"))) {
        setDefaultSettings();
    }
}

void setSetting(const char *key, float value) {
    const char *abbreviation = resolveToAbbreviation(key);
    String filePath = getSettingFilePath(abbreviation);

    fs::File file = LittleFS.open(filePath, "w");
    if (file) {
        file.printf("%.6f", value);
        file.close();
    }
}

float getSetting(const char *key) {
    const char *abbreviation = resolveToAbbreviation(key);
    String filePath = getSettingFilePath(abbreviation);

    fs::File file = LittleFS.open(filePath, "r");
    if (file) {
        String valueStr = file.readString();
        file.close();
        return valueStr.toFloat();
    }

    return -1.0;
}
