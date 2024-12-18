#include "config.h"

// tasks.cpp
extern void sprint(const char* format, ...);
extern void audioTask(void* param);
extern void transmitTask(void* param);
extern void receiveTask(void* param);

// audio.cpp
extern void setupAudio();

SPIClass spi(VSPI);
SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE0);

SX1262 radio = new Module(CS_PIN, IRQ_PIN, RESET_PIN, RADIOLIB_NC, spi, spiSettings);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  sprint("\n\nMAIN:setup_start");

  // SPI setup
  spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  // LoRa initialization
  sprint("MAIN:lora_init");
  int state = radio.begin(LORA_FREQ, LORA_BW, LORA_SF, LORA_CR, LORA_SW, LORA_POWER, LORA_PL, 0, false);
  if (state != RADIOLIB_ERR_NONE) {
    sprint("MAIN:lora_init_fail:0x%x", state);
    sprint("MAIN:setup:hang");
    while (true) { delay(10); }
  }
  radio.setCRC(LORA_CRC);
  sprint("MAIN:lora_init_succ");

  setupAudio();

  // run audio loopback on a separate task
  xTaskCreate(&audioTask,     "AudioTask",    AUDIO_TASK_STACK_SIZE,    NULL, 1, NULL);
  xTaskCreate(&transmitTask,  "TransmitTask", TRANSMIT_TASK_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(&receiveTask,   "ReceiveTask",  TRANSMIT_TASK_STACK_SIZE, NULL, 1, NULL);

  sprint("MAIN:setup_done");
}

void loop() {
  delay(1000);
}