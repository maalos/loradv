#include <config.h>
#include <TJpg_Decoder.h>
#include "maps.h"

#define WIFI_SSID "bigblackballs"
#define WIFI_PASS "bigblackballs"

#define MAX_IMAGE_WIDTH 240
#define FORMAT_LITTLEFS_IF_FAILED true // gotta check preferences.cpp

HTTPClient http;
String apiUrl = "https://65731956-dd67-4a69-96f8-6a3337a1d780-00-ck5x0o80o03o.riker.replit.dev/smallgmap?maptype=hybrid&lat=50.000000&lon=15.000000&zoom=18";

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    if (y >= tft.height())
        return 0;

    tft.pushImage(x, y, w, h, bitmap);

    return 1;
}

void connectToWiFi()
{
    Serial.print("Connecting to WiFi");
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(".");
    }

    Serial.println("Connected to WiFi");
}

bool mapsAppInitialized = false;
void mapsApp()
{
    if (!mapsAppInitialized)
    {
        Serial.println("initialising app");
        // tft.fillScreen(TFT_BLACK);
        // tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);
        //if (!LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED))
        //{
        //    Serial.println("LittleFS initialisation failed!");
        //    while (1)
        //        vPortYield(); // Stay here twiddling thumbs waiting
        //}

        TJpgDec.setJpgScale(1);

        // The byte order can be swapped (set true for TFT_eSPI)
        TJpgDec.setSwapBytes(true);

        // The decoder must be given the exact name of the rendering function above
        TJpgDec.setCallback(tft_output);
        mapsAppInitialized = true;
    }

    uint32_t t = millis();

    bool loaded_ok = getFile(apiUrl, "/map.jpg", true);
    t = millis() - t;

    if (loaded_ok)
    {
        Serial.printf("Took %dms to download\n", t);
    }

    t = millis();

    Serial.println(TJpgDec.drawFsJpg(0, 0, "/map.jpg", LittleFS));

    t = millis() - t;
    Serial.printf("Took %dms to draw\n", t);

    vTaskDelay(10000 / portTICK_PERIOD_MS);
}
