#include <config.h>

TFT_eSPI tft = TFT_eSPI();
TaskHandle_t displayTaskHandle;

int16_t ttf_width = tft.width();
int16_t ttf_halfwidth = ttf_width / 2;
int16_t ttf_height = tft.height();
int16_t ttf_halfheight = ttf_height / 2;

bool appStopSignal = false;

typedef std::pair<const char *, std::function<void()>> appPair_t;

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
    if (CODEC2_MODE == CODEC2_MODE_3200)
        return "3200";
    if (CODEC2_MODE == CODEC2_MODE_2400)
        return "2400";
    if (CODEC2_MODE == CODEC2_MODE_1600)
        return "1600";
    if (CODEC2_MODE == CODEC2_MODE_1400)
        return "1400";
    if (CODEC2_MODE == CODEC2_MODE_1300)
        return "1300";
    if (CODEC2_MODE == CODEC2_MODE_1200)
        return "1200";
    if (CODEC2_MODE == CODEC2_MODE_700C)
        return "700C";
    return "WHAT";
}

std::map<const char *, std::function<void()>> appMap = {
    {"VFO/CH (home)  ", vfoApp},
    {"Settings       ", settingsApp},
    {"LoRaMaps       ", mapsApp},
};

appPair_t findAppPairByName(const char *name)
{
    auto it = appMap.find(name);
    if (it != appMap.end())
    {
        return *it;
    }

    return appPair_t{"", nullptr};
}

char array[18];

void displayTask(void *param)
{
    LOG_INFO("Display task started");

    appPair_t currentAppPair = findAppPairByName("LoRaMaps       ");

    while (true)
    {
        currentAppPair.second();

        vTaskDelay(250 / portTICK_PERIOD_MS); // update it every 250ms
    }

    LOG_INFO("Display task done");
    vTaskDelete(NULL);
}

void setupDisplay()
{
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(DISP_BGCOLOR);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_52);
    tft.drawString("loraDV", ttf_halfwidth, ttf_halfheight);
    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24); // reset font
}