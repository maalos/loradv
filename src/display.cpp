#include <config.h>

TFT_eSPI tft = TFT_eSPI();
TaskHandle_t displayTaskHandle;

int16_t ttf_width = tft.width();
int16_t ttf_halfwidth = ttf_width / 2;
int16_t ttf_height = tft.height();
int16_t ttf_halfheight = ttf_height / 2;

bool appStopSignal = false;
unsigned short currentAppIndex = 2;

struct App
{
    char id;
    const char *name;
    void (*function)();
    void (*encoderHandler)(bool, int); // -1 = ccw, 0 = no rotation, +1 = cw
};

void defaultEncoderHandler(bool pressed, int value) {};

App appList[] = {
    {1, "VFO/CH (home)  ", vfoApp, defaultEncoderHandler},
    {2, "Settings       ", settingsApp, settingsEncoderHandler},
    {3, "LoRaMaps       ", mapsApp, defaultEncoderHandler},
    {0, nullptr, nullptr, nullptr} // end of the list
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

void triggerEncoderHandler(bool pressed, int value) {
    App currentApp = getAppById(currentAppIndex);
    
    if (currentApp.encoderHandler) {
        currentApp.encoderHandler(pressed, value);
    }

}

void displayTask(void *param)
{
    Serial.println(F("Display task started"));

    App currentApp = getAppById(currentAppIndex);

    while (true)
    {
        if (currentApp.function)
        {
            currentApp.function();
        }

        vTaskDelay(50 / portTICK_PERIOD_MS); // update it every x ms
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
