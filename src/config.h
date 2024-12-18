#ifndef CONFIG_H
#define CONFIG_H

// Includes
#include <driver/i2s.h>
#include <codec2.h>
#include <RadioLib.h>
#include <SPI.h>
#include <CircularBuffer.hpp>
#include <Arduino.h>

// Serial
#define SERIAL_BAUD_RATE    9600

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
// LoRa params
#define LORA_FREQ   434.0F  // initial frequency
#define LORA_BW     125.0F  // bandwidth in kHz
#define LORA_SF     9U      // spreading factor // 5U
#define LORA_CR     7U      // coding rate denominator
#define LORA_SW     12U     // sync word
#define LORA_POWER  10      // this one's signed - output power in dBm
#define LORA_PL     8U      // preamble length
#define LORA_CRC    1U      // length of the CRC in bytes

// Tasks
#define AUDIO_TASK_STACK_SIZE       16384*2 // Not sure if I can decrease the footprint here
#define TRANSMIT_TASK_STACK_SIZE    2048    // I reckon that's around the minimum size
#define BUFFER_SIZE 512                     // Size of circular buffer - has to be this big to send 3200kb/s

#endif // CONFIG_H