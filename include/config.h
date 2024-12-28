#ifndef CONFIG_H
#define CONFIG_H

// Includes
#include <RadioLib.h>
#include <driver/i2s.h>
#include <codec2.h>
#include <CircularBuffer.hpp>
#include <arduino-timer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <DejaVu_Sans_Mono_Bold_24.h>
#include <AiEsp32RotaryEncoder.h>
#include <Preferences.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LittleFS.h>

// GPIO 26, 4 free

extern SX1262 radio;
#define SERIAL_BAUD_RATE    9600
#define PTTBTN_PIN      36 // VP
extern volatile bool pttPressed;
extern volatile char radioAction;

// encoder.cpp
#define ROTARY_ENCODER_A_PIN 34
#define ROTARY_ENCODER_B_PIN 39     // VN
#define ROTARY_ENCODER_BUTTON_PIN   33
extern TaskHandle_t encoderTaskHandle;
extern void encoderTask(void *param);
extern void setupEncoder();

// sleep.cpp
//#define ENABLE_SLEEP
#define SLEEP_DELAY_MS  10000  // how long to wait before entering sleep
#define PIN_TO_BITMASK(GPIO) digitalPinToInterrupt((1ULL << GPIO))
#define SLEEP_BITMASK   PIN_TO_BITMASK(PTTBTN_PIN) | PIN_TO_BITMASK(LORA_RADIO_PIN_B) // | PIN_TO_BITMASK(ROTARY_ENCODER_A_PIN) | PIN_TO_BITMASK(ROTARY_ENCODER_B_PIN) | PIN_TO_BITMASK(ROTARY_ENCODER_BUTTON_PIN)
extern Timer<1> sleepTimer;
extern bool sleep(void *param);
extern void sleepReset();
extern void onLoraDataAvailableIsr();

// audio.cpp
// Audio
#define AUDIO_I2S_SCK   14
// Speaker
#define AUDIO_SPEAKER_LRC   13
#define AUDIO_SPEAKER_DIN   25
// Microphone
#define AUDIO_MIC_SD    32
#define AUDIO_MIC_WS    15
// Sample rate (don't change)
#define AUDIO_SAMPLE_RATE   8000
// Codec2-related
#define CODEC2_MODE             CODEC2_MODE_1200
#define CODEC2_LPC_PF_ENABLE    1   // decent audio quality increase
#define CODEC2_LPC_PF_BASSBOOST 0   // don't use this
#define CODEC2_LPC_PF_BETA      0.2 // [0.8] in sh123/loradv, new values are from
#define CODEC2_LPC_PF_GAMMA     0.5 // [0.2]                  drowe67/codec2/doc/codec2.pdf
#define AUDIO_TASK_PLAY_BIT         0x01    // task bit flag to start playback
#define AUDIO_TASK_RECORD_BIT       0x02    // task bit flag to start recording
extern TaskHandle_t audioTaskHandle;
extern struct CODEC2 *c2;
extern int c2_samples_per_frame;
extern int c2_bytes_per_frame;
extern int16_t *c2_samples;
extern uint8_t *c2_bits;
extern void audioTask(void *param);
extern void setupAudio();

// preferences.cpp
#define SETTINGS_DIR F("/settings")
struct Setting {
    const char *abbreviation;
    const char *fullKey;
    float defaultValue;
};
extern String getSettingFilePath(const char *abbreviation);
extern const Setting defaultSettings[];
extern const char *resolveToAbbreviation(const char *key);
extern const char *resolveToFullKey(const char *abbreviation);

extern void setDefaultSettings();
extern void setupPreferences();
extern void setSetting(const char* key, float value);
extern float getSetting(const char* key);

// lora.cpp
#define LORA_RADIO_TASK_RX_BIT      0x01    // lora task rx bit command
#define LORA_RADIO_TASK_TX_BIT      0x02    // lora task tx bit command
#define LORA_RADIO_BUF_LEN      256   // packets buffer size
#define LORA_RADIO_QUEUE_LEN    512   // queues length
// VSPI pin definitions + E220-400M30S labels
#define LORA_RADIO_PIN_SS   SS  // 5 - NSS 19
#define SCK_PIN             18  // SCK 18
#define MISO_PIN            19  // MISO 16
#define MOSI_PIN            23  // MOSI 17
#define LORA_RADIO_PIN_A    17  // DIO1 13
#define LORA_RADIO_PIN_B    35  // BUSY 14
#define LORA_RADIO_PIN_RST  22  // NRST 15
#define LORA_RADIO_PIN_RXEN 2   // RXEN 6
#define LORA_RADIO_PIN_TXEN 0   // TXEN 7
// LoRa params
#define LORA_RADIO_FREQ 434.0   // initial frequency
#define LORA_RADIO_BW   31.25   // bandwidth in kHz
#define LORA_RADIO_SF   7       // spreading factor (SF12-10 is too slow for DV)
#define LORA_RADIO_CR   5       // coding rate denominator
#define LORA_RADIO_SYNC 8       // sync word
#define LORA_RADIO_PWR  2       // power in dbm (real is +10db if module has amplifier)
#define LORA_RADIO_PL   12      // preamble length
#define LORA_RADIO_CRC  1       // length of the CRC in bytes
#define LORA_RADIO_EXPL         // comment out to use implicit mode (for spreading factor 6)
extern TaskHandle_t loraTaskHandle;
extern int state;
extern void loraTask(void *param);
// lora task packet and packet index/size queues
extern CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_rx_queue;
extern CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_rx_queue_index;
extern CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_tx_queue;
extern CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_tx_queue_index;
// packet buffers
extern byte lora_radio_rx_buf[LORA_RADIO_BUF_LEN];  // tx packet buffer
extern byte lora_radio_tx_buf[LORA_RADIO_BUF_LEN];  // rx packet buffer
extern float getFrequency();
extern void setFrequency(float freq);
extern float getBandwidth();
extern void setBandwidth(float bw);
extern float getSpreadingFactor();
extern void setSpreadingFactor(float sf);
extern float getCodingRate();
extern void setCodingRate(float cr);
extern float getSyncWord();
extern void setSyncWord(float sw);
extern float getOutputPower();
extern void setOutputPower(float txp);
extern float getPreambleLength();
extern void setPreambleLength(float pl);
extern float getCrcLength();
extern void setCrcLength(float crcl);


// display.cpp
#define DISP_BGCOLOR 0x2009
#define DISPLAY_BACKLIGHT_PIN 16
extern TFT_eSPI tft;
extern const char *rssiToSValue(short rssi);
extern const char *c2ToString();
extern TaskHandle_t displayTaskHandle;
extern int16_t ttf_width;
extern int16_t ttf_halfwidth;
extern int16_t ttf_height;
extern int16_t ttf_halfheight;
extern bool appStopSignal;
extern void updateStringAt(uint8_t x, uint8_t y, const char *text, int fgColor);
extern char array[18];
extern void displayTask(void *param);
extern void setupDisplay();


// apps/vfo.cpp
extern void vfoApp();

// apps/settings.cpp
extern void settingsApp();

// apps/maps.cpp
extern void connectToWiFi();
extern void mapsApp();

/*
GOOD CONFIGS

700C@20.8:SF7:CR6   best bw
700C@31.25:SF8:CR5  best rsens // i'd use this 
700C@41.7:SF8:CR7   best cr

1200@31.25:SF7:CR5  best bw and rsens // i'd use this 
1200@125:SF9:CR7    best cr

1600@41.7:SF7:CR5   smallest bw and 2nd best rsens // i'd use this 
1600@62.5:SF7:CR8   best cr
1600@125:SF9:CR5    best rsens but meh bw

2400@62.5:SF7:CR5   best rsens and bw // i'd use this
2400@125:SF8:CR6    best cr

3200@125:SF7:CR8    best cr // poor hams lol, i'd use this
3200@250:SF9:CR5    best rsens
*/

#endif // CONFIG_H