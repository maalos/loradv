#include <config.h>

TFT_eSPI tft = TFT_eSPI();

int16_t ttf_width = tft.width();
int16_t ttf_halfwidth = ttf_width / 2;
int16_t ttf_height = tft.height();
int16_t ttf_halfheight = ttf_height / 2;

void updateStringAt(uint8_t x, uint8_t y, const char *text, int fgColor)
{
    tft.setTextColor(fgColor, LDV_PURPLE, true);
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

const char *c2ToString() {
    if (CODEC2_MODE == CODEC2_MODE_3200) return "3200";
    if (CODEC2_MODE == CODEC2_MODE_2400) return "2400";
    if (CODEC2_MODE == CODEC2_MODE_1600) return "1600";
    if (CODEC2_MODE == CODEC2_MODE_1400) return "1400";
    if (CODEC2_MODE == CODEC2_MODE_1300) return "1300";
    if (CODEC2_MODE == CODEC2_MODE_1200) return "1200";
    if (CODEC2_MODE == CODEC2_MODE_700C) return "700C";
    return "WHAT";
}

char array[18];

void display_task(void *param)
{
    LOG_INFO("Display task started");
    short rssi;
    float snr;

    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);

    while (true)
    {
        rssi = radio.getRSSI();
        snr = radio.getSNR();

        switch (radioAction)
        {
        case 2:
            updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "TX ", TFT_RED);
            break;

        case 1:
            // row 1
            updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "RX ", TFT_GREEN);
            // row 5
            sprintf(array, "          %s", rssiToSValue(rssi));
            updateStringAt(ttf_halfwidth, ttf_halfheight + 40, array, TFT_GREEN);

            sprintf(array, "%ddB", rssi);
            updateStringAt(ttf_halfwidth, ttf_halfheight + 40, array, TFT_GREEN);

            sprintf(array, "SNR: %.01f", snr);
            updateStringAt(ttf_halfwidth, ttf_halfheight + 70, array, TFT_GREEN);
            break;
        default:
            updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "LI ", TFT_DARKGREY);
            updateStringAt(ttf_halfwidth, ttf_halfheight + 40, "                 ", TFT_GREEN);
            updateStringAt(ttf_halfwidth, ttf_halfheight + 70, "                 ", TFT_GREEN);
            break;
        }

        vTaskDelay(500 / portTICK_PERIOD_MS); // update it every 250ms
    }

    LOG_INFO("Display task done");
    vTaskDelete(NULL);
}

void setupDisplay()
{
    tft.init();
    tft.setRotation(3);
    tft.fillScreen(LDV_PURPLE);
    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);

    // row 1
    updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "            100%", TFT_GREEN);
    updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "     4.19V", TFT_GREEN);
    updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "LI", TFT_DARKGREY);

    // tft.drawLine(0, ttf_halfheight - 85, ttf_width, ttf_halfheight - 85,    TFT_WHITE);

    // row 2
    sprintf(array, "           %ddBm", LORA_RADIO_PWR + 10); // +10dBm because of the PA
    updateStringAt(ttf_halfwidth, ttf_halfheight - 70, array, TFT_RED);

    sprintf(array, "    %.01fkHz", LORA_RADIO_BW);
    updateStringAt(ttf_halfwidth, ttf_halfheight - 70, array, TFT_ORANGE);

    sprintf(array, "SF%d", LORA_RADIO_SF);
    updateStringAt(ttf_halfwidth, ttf_halfheight - 70, "SF7", TFT_ORANGE);

    // row 3
    sprintf(array, "            %s", c2ToString());
    updateStringAt(ttf_halfwidth, ttf_halfheight - 40, array, TFT_ORANGE);

    updateStringAt(ttf_halfwidth, ttf_halfheight - 40, "CH1", TFT_WHITE);

    // freq - row 4
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.setFreeFont(&DejaVu_Sans_Mono_Bold_52);
    tft.drawString("434.000", ttf_halfwidth, ttf_halfheight);
}