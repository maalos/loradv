#ifndef CONFIG_H
#define CONFIG_H

// Includes
#include <DebugLog.h>
#include <RadioLib.h>
#include <driver/i2s.h>
#include <codec2.h>
#include <CircularBuffer.hpp>
#include <arduino-timer.h>

// Serial
#define SERIAL_BAUD_RATE    9600

// PTT button
#define PTTBTN_PIN      36 // VP
#define PTTBTN_GPIO_PIN GPIO_NUM_12

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
#define CODEC2_MODE             CODEC2_MODE_3200
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
#define LORA_RADIO_PIN_B    35  // BUSY 14 // warning here?
#define LORA_RADIO_PIN_RST  22  // NRST 15
#define LORA_RADIO_PIN_RXEN 2   // RXEN 6
#define LORA_RADIO_PIN_TXEN 0   // TXEN 7
// LoRa params
#define LORA_RADIO_FREQ 434.0   // initial frequency
#define LORA_RADIO_BW   125.0   // bandwidth in kHz
#define LORA_RADIO_SF   7       // spreading factor (SF12-10 is too slow for DV)
#define LORA_RADIO_CR   7       // coding rate denominator
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
KNOWN GOOD CONFIGS

700C@125kHz:SF9

1200@125kHz:SF8
1600@125kHz:SF8

2400@125kHz:SF7
3200@125kHz:SF7
*/