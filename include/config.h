#ifndef CONFIG_H
#define CONFIG_H

// Includes
#include <DebugLog.h>
#include <RadioLib.h>
#include <driver/i2s.h>
#include <codec2.h>
#include <CircularBuffer.hpp>
#include <arduino-timer.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <DejaVu_Sans_Mono_Bold_24.h>
#include <DejaVu_Sans_Mono_Bold_52.h>

// Display
#define LDV_PURPLE 0x2009

// Externs
extern SX1262 radio;

extern volatile bool ptt_pressed;
extern volatile char radioAction;

extern Timer<1> light_sleep_timer;
extern bool light_sleep(void *param);
extern void light_sleep_reset();
extern void onLoraDataAvailableIsr();

extern TaskHandle_t audio_task_handle;
extern void audio_task(void *param);
extern void setupAudio();

extern TaskHandle_t lora_task_handle;
extern void lora_task(void *param);

extern TaskHandle_t display_task_handle;
extern void display_task(void *param);
extern void setupDisplay();

#define ENABLE_DISPLAY

// Serial
#define SERIAL_BAUD_RATE    9600

// PTT button
#define PTTBTN_PIN      36 // VP
#define PTTBTN_GPIO_PIN GPIO_NUM_36 // vp

// Misc
//#define ENABLE_LIGHT_SLEEP      true
#define LIGHT_SLEEP_DELAY_MS    5000  // how long to wait before engering light sleep
#define LIGHT_SLEEP_BITMASK     (uint64_t)(1 << LORA_RADIO_PIN_B) // bit mask for ext1 high pin wake up

// Audio
// Speaker
#define AUDIO_SPEAKER_BCLK  26
#define AUDIO_SPEAKER_LRC   13
#define AUDIO_SPEAKER_DIN   25
// Microphone
#define AUDIO_MIC_SD    32
#define AUDIO_MIC_WS    15
#define AUDIO_MIC_SCK   14
// Sample rate (don't change)
#define AUDIO_SAMPLE_RATE   8000
// Codec2-related
#define CODEC2_MODE             CODEC2_MODE_1200
#define CODEC2_LPC_PF_ENABLE    1   // decent audio quality increase
#define CODEC2_LPC_PF_BASSBOOST 0   // don't use this
#define CODEC2_LPC_PF_BETA      0.2 // [0.8] in sh123/loradv, new values are from
#define CODEC2_LPC_PF_GAMMA     0.5 // [0.2]                  drowe67/codec2/doc/codec2.pdf

// Radio-related
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
#define LORA_RADIO_SYNC 0x34    // sync word
#define LORA_RADIO_PWR  2       // power in dbm (real is +10db if module has amplifier)
#define LORA_RADIO_PL   4U      // preamble length
#define LORA_RADIO_CRC  1       // length of the CRC in bytes
#define LORA_RADIO_EXPL true    // comment out to use implicit mode (for spreading factor 6)

// Queues and buffers
#define LORA_RADIO_BUF_LEN      256   // packets buffer size
#define LORA_RADIO_QUEUE_LEN    512   // queues length

// Tasks
#define LORA_RADIO_TASK_RX_BIT      0x01    // lora task rx bit command
#define LORA_RADIO_TASK_TX_BIT      0x02    // lora task tx bit command
#define AUDIO_TASK_PLAY_BIT         0x01    // task bit flag to start playback
#define AUDIO_TASK_RECORD_BIT       0x02    // task bit flag to start recording

#endif // CONFIG_H


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