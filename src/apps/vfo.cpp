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
        tft.fillScreen(DISP_BGCOLOR);
        tft.setFreeFont(&DejaVu_Sans_Mono_Bold_24);
        tft.setTextSize(1);
        
        // row 1
        updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "            100%", TFT_GREEN);
        updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "     4.19V", TFT_GREEN);
        updateStringAt(ttf_halfwidth, ttf_halfheight - 100, "LI", TFT_DARKGREY);

        // tft.drawLine(0, ttf_halfheight - 85, ttf_width, ttf_halfheight - 85,    TFT_WHITE);

        // row 2
        sprintf(array, "           %2ddBm", LORA_RADIO_PWR + 10); // +10dBm because of the PA
        updateStringAt(ttf_halfwidth, ttf_halfheight - 70, array, TFT_RED);

        sprintf(array, "    %.01fkHz", LORA_RADIO_BW);
        updateStringAt(ttf_halfwidth, ttf_halfheight - 70, array, TFT_ORANGE);

        sprintf(array, "SF%d", LORA_RADIO_SF);
        updateStringAt(ttf_halfwidth, ttf_halfheight - 70, "SF7", TFT_ORANGE);

        // row 3
        sprintf(array, "            %s", c2ToString());
        updateStringAt(ttf_halfwidth, ttf_halfheight - 40, array, TFT_ORANGE);

        updateStringAt(ttf_halfwidth, ttf_halfheight - 40, "CH1", TFT_WHITE);
        
        vfoAppInitialized = true;
    }
    newFrequency = getFrequency();
    if (newFrequency != previousFrequency)
    {
        tft.setTextDatum(MC_DATUM);
        tft.setTextColor(TFT_WHITE);
        //tft.setFreeFont(&DejaVu_Sans_Mono_Bold_52);
        tft.setTextSize(2);
        tft.drawString(String(newFrequency, 3), ttf_halfwidth, ttf_halfheight);
        tft.setTextSize(1); // reset font
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