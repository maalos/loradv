#include <config.h>
#include <FS.h>
#include <LittleFS.h>

bool settingsAppInitialized = false;

void settingsApp()
{
    if (!settingsAppInitialized)
    {
        tft.setTextSize(1);
        settingsAppInitialized = true;
    }

    Serial.println(F("Printing all settings:"));

    // Ensure settings directory exists
    if (!LittleFS.exists(SETTINGS_DIR))
    {
        Serial.println(F("Settings directory does not exist"));
        return;
    }

    /*
    // Open the settings directory
    Dir dir = LittleFS.openDir(SETTINGS_DIR); // identifier "Dir" is undefinedC/C++(20)
    while (dir.next())
    {
        String fileName = dir.fileName();
        String filePath = getSettingFilePath(fileName.c_str());

        // Get the full key corresponding to the abbreviation
        const char *fullKey = resolveToFullKey(fileName.c_str());

        fs::File file = LittleFS.open(filePath, "r");
        if (file)
        {
            String valueStr = file.readString();
            file.close();
            float value = valueStr.toFloat();

            // Print the full key and value
            Serial.print(fullKey);
            Serial.print(": ");
            Serial.println(value, 6);
        }
    }
    */
}