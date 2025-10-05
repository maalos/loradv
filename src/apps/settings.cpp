#include <config.h>
#include <FS.h>
#include <LittleFS.h>

bool settingsAppInitialized = false;


void redrawCursor(char row) {
    tft.fillRect(0, 24, 16, TFT_HEIGHT, DISP_BGCOLOR);
    tft.drawString(">", 0, 24 * (row + 1));
}

void settingsEncoderHandler(bool pressed, int value) {
    Serial.printf("settingsEncoderHandler: %d %d\n", pressed, value);
};

#define MAX_SCREEN_ROWS 10

void settingsApp()
{
    if (!settingsAppInitialized)
    {
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE);
        tft.setTextDatum(TL_DATUM);
        tft.fillScreen(DISP_BGCOLOR);
        encoder0Pos = 0;
        redrawCursor(encoder0Pos);
        settingsAppInitialized = true;
    }

    tft.drawString("Settings", 0, 0);
    for (int i = 0; i < MAX_SCREEN_ROWS - 1; i++) {
        const Setting &setting = defaultSettings[i];
        tft.drawString(String(" ") + setting.fullKey, 0, 24 * (i + 1));
    }

    redrawCursor(encoder0Pos);

    // Serial.println(F("Printing all settings:"));

    // Ensure settings directory exists
    // if (!LittleFS.exists(SETTINGS_DIR))
    // {
        // Serial.println(F("Settings directory does not exist"));
        // return;
    // }

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