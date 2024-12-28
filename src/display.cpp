#include <config.h>

TFT_eSPI tft = TFT_eSPI();
TaskHandle_t displayTaskHandle;

int16_t ttf_width = tft.width();
int16_t ttf_halfwidth = ttf_width / 2;
int16_t ttf_height = tft.height();
int16_t ttf_halfheight = ttf_height / 2;

bool appStopSignal = false;

struct App
{
    char id;
    const char *name;
    void (*function)();
};

App appList[] = {
    {1, "VFO/CH (home)  ", vfoApp},
    {2, "Settings       ", settingsApp},
    {3, "LoRaMaps       ", mapsApp},
    {0, nullptr, nullptr} // end of the list
};

App getAppById(int id)
{
    for (int i = 0; appList[i].id != 0; ++i)
    {
        if (appList[i].id == id)
        {
            return appList[i];
        }
    }
    return {0, nullptr, nullptr}; // return end of the list if not found
}

void updateStringAt(uint8_t x, uint8_t y, const char *text, int fgColor)
{
    tft.setTextColor(fgColor, DISP_BGCOLOR, true);
    tft.setTextDatum(ML_DATUM);
    tft.drawString(text, 0, y);
    tft.setTextColor(TFT_WHITE);
}

const char *rssiToSValue(short rssi)
{
    rssi += 160; // idk why

    if (rssi >= -73)
        return "   S9+";
    if (rssi >= -93)
        return "    S9";
    if (rssi >= -103)
        return "    S8";
    if (rssi >= -113)
        return "    S7";
    if (rssi >= -123)
        return "    S6";
    if (rssi >= -133)
        return "    S5";
    if (rssi >= -143)
        return "    S4";
    if (rssi >= -153)
        return "    S3";
    if (rssi >= -163)
        return "    S2";
    if (rssi >= -173)
        return "    S1";

    return "    S0";
}

const char *c2ToString()
{
    static const char *codec2Modes[] = {
        "3200", "2400", "1600", "1400", "1300", "1200", nullptr, nullptr, "700C"};

    if (CODEC2_MODE >= 0 && CODEC2_MODE <= 8)
    {
        return codec2Modes[CODEC2_MODE];
    }

    return "WHAT";
}

char array[18];

void displayTask(void *param)
{
    Serial.println(F("Display task started"));

    App currentApp = getAppById(2);

    while (true)
    {
        if (currentApp.function)
        {
            currentApp.function();
        }

        vTaskDelay(250 / portTICK_PERIOD_MS); // update it every 250ms
    }

    Serial.println(F("Display task done"));
    vTaskDelete(NULL);
}

void setupDisplay()
{
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(DISP_BGCOLOR);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);
    tft.setTextSize(2);
    tft.drawString("loraDV", ttf_halfwidth, ttf_halfheight);
    tft.setTextSize(1);
}
