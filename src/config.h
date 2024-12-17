#ifndef CONFIG_H
#define CONFIG_H

// Includes
#include <driver/i2s.h>
#include <codec2.h>
#include <RadioLib.h>
#include <SPI.h>
#include <CircularBuffer.h>
#include <Arduino.h>

#define SPRINT Serial.println

// Serial configuration
#define SERIAL_BAUD_RATE    9600

// Audio speaker configuration
#define AUDIO_SPEAKER_BCLK  26
#define AUDIO_SPEAKER_LRC   13
#define AUDIO_SPEAKER_DIN   25

// Audio microphone configuration
#define AUDIO_MIC_SD    32
#define AUDIO_MIC_WS    15
#define AUDIO_MIC_SCK   14

// Audio sample rate
#define AUDIO_SAMPLE_RATE   8000

#define CODEC CODEC2_MODE_1200
#define CODEC2_LPC_PF_ENABLE    1   // decent audio quality increase
#define CODEC2_LPC_PF_BASSBOOST 0
#define CODEC2_LPC_PF_BETA      0.2 // [0.8] in sh123/loradv, new values are from
#define CODEC2_LPC_PF_GAMMA     0.5 // [0.2]                  drowe67/codec2/doc/codec2.pdf

// Radio-related
// VSPI pin definitions + E220-400M30S labels
#define CS_PIN      5   // NSS 19
#define SCK_PIN     18  // SCK 18
#define MISO_PIN    19  // MISO 16
#define MOSI_PIN    23  // MOSI 17

#define IRQ_PIN     17  // DIO1 13
#define RESET_PIN   22  // NRST 15

#define BUFFER_SIZE 64  // Size of circular buffer

#endif // CONFIG_H