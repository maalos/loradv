#include <config.h>

TFT_eSPI tft = TFT_eSPI();

int16_t ttf_width       = tft.width();
int16_t ttf_halfwidth   = ttf_width / 2;
int16_t ttf_height      = tft.height();
int16_t ttf_halfheight  = ttf_height / 2;

void updateStringAt(uint8_t x, uint8_t y, const char *text, int fgColor) {
  tft.setTextColor(fgColor, LDV_PURPLE, true);
  tft.setTextDatum(ML_DATUM);
  tft.drawString(text, 0, y);
  tft.setTextColor(TFT_WHITE);
}

char* rssiToSValue(float rssi) {
    static char result[16]; // Static to persist between function calls

    rssi += 160; // idk why

    if (rssi >= -73) {
        int dBOverS9 = static_cast<int>(-73 - rssi); // dB over S9
        sprintf(result, "S9+%ddB", dBOverS9);
        return result;
    }
    if (rssi >= -93)  return "    S9";
    if (rssi >= -103) return "    S8";
    if (rssi >= -113) return "    S7";
    if (rssi >= -123) return "    S6";
    if (rssi >= -133) return "    S5";
    if (rssi >= -143) return "    S4";
    if (rssi >= -153) return "    S3";
    if (rssi >= -163) return "    S2";
    if (rssi >= -173) return "    S1";

    return "S0";
}

void display_task(void *param) {
    LOG_INFO("Display task started");
    short rssi;
    char array[3];

    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);

    while(true) {
        rssi = radio.getRSSI();
        array[3];

        switch (radioAction) {
            case 2:
                updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "TX ",              TFT_RED);
                break;

            case 1:
                // row 1
                updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "RX ",              TFT_GREEN);
                // row 5
                //updateStringAt(ttf_halfwidth, ttf_halfheight + 40,  strcat("          ", rssiToSValue(rssi)), TFT_GREEN);
                sprintf(array, "%ddB", rssi);
                updateStringAt(ttf_halfwidth, ttf_halfheight + 40,  array,              TFT_GREEN);
                break;
            default:
                updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "LI ",              TFT_DARKGREY);
                updateStringAt(ttf_halfwidth, ttf_halfheight + 40,  "                ", TFT_GREEN);
                break;
        }

        vTaskDelay(100 / portTICK_PERIOD_MS); // update it every 250ms
    }

    LOG_INFO("Display task done");
    vTaskDelete(NULL);
}

void setupDisplay() {
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(LDV_PURPLE);
  tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);

  // row 1
  updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "            100%", TFT_GREEN);
  updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "     4.19V",       TFT_GREEN);
  updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "LI",               TFT_DARKGREY);

  //tft.drawLine(0, ttf_halfheight - 85, ttf_width, ttf_halfheight - 85,    TFT_WHITE);

  // row 2
  updateStringAt(ttf_halfwidth, ttf_halfheight - 70,  "           30dBm", TFT_RED);
  updateStringAt(ttf_halfwidth, ttf_halfheight - 70,  "    125kHz",       TFT_ORANGE);
  updateStringAt(ttf_halfwidth, ttf_halfheight - 70,  "SF7",              TFT_ORANGE);

  // row 3
  updateStringAt(ttf_halfwidth, ttf_halfheight - 40,  "           3200k", TFT_ORANGE);
  updateStringAt(ttf_halfwidth, ttf_halfheight - 40,  "CH1",              TFT_WHITE);

  // freq - row 4
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(&DejaVu_Sans_Mono_Bold_52);
  tft.drawString("434.000", ttf_halfwidth, ttf_halfheight);
}