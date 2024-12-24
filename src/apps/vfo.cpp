#include <config.h>

volatile short rssi;
volatile float snr;
volatile bool vfoAppInitialized = false;
volatile float previousFrequency = getFrequency();
volatile float newFrequency = 0;

void vfoApp()
{
    if (!vfoAppInitialized)
    {
        tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);
        vfoAppInitialized = true;
    }
    newFrequency = getFrequency();
    if (newFrequency != previousFrequency) {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE);
        tft.setFreeFont(&DejaVu_Sans_Mono_Bold_52);
        tft.drawString(String(newFrequency, 3), ttf_halfwidth, ttf_halfheight);
        tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24); // reset font
        previousFrequency = newFrequency;
    }

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
}