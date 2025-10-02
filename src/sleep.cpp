#include <config.h>

TimerHandle_t sleepTimer;

// called by FreeRTOS timer to enter sleep
void sleepCallback(TimerHandle_t xTimer)
{
#ifdef ENABLE_SLEEP
    radioAction = 0;
    Serial.println(F("Entering sleep"));
    digitalWrite(TFT_BL, LOW);

    // https://www.reddit.com/r/esp32/comments/movnp3/comment/hlad7wh/
    tft.writecommand(ST7789_DISPOFF);
    delay(100);
    tft.writecommand(ST7789_SLPIN);

    // wake up on PTT button or LoRa radio incoming data
    esp_sleep_enable_ext1_wakeup(SLEEP_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
    delay(100);
    esp_light_sleep_start();

    Serial.println(F("Exiting sleep"));
    
    tft.writecommand(ST7789_SLPOUT);
    delay(100);
    tft.writecommand(ST7789_DISPON);

    digitalWrite(TFT_BL, HIGH);
#endif
}

// start/reset sleep timer
void sleepReset(String reason)
{
#ifdef ENABLE_SLEEP
    Serial.printf("sleepReset reason: %s\n", reason.c_str());

    if (sleepTimer != NULL) {
        // Stop and restart the timer
        xTimerStop(sleepTimer, 0);
        xTimerChangePeriod(sleepTimer, pdMS_TO_TICKS(SLEEP_DELAY_MS), 0);
        xTimerStart(sleepTimer, 0);
    }
#endif
}
