#include "config.h"

extern void audioTask(void* param);
extern void transmitTask(void* param);

SPIClass spi(VSPI);
SPISettings spiSettings(2000000, MSBFIRST, SPI_MODE0);

SX1262 radio = new Module(CS_PIN, IRQ_PIN, RESET_PIN, RADIOLIB_NC, spi, spiSettings);

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  SPRINT("MAIN:setup_start");

  // SPI setup
  spi.begin(SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN);

  // LoRa initialization
  SPRINT("MAIN:lora_init");
  int state = radio.begin(434.0F, 125.0F, 9U, 7U, 12U, 10, 8U, 0, false);
  if (state != RADIOLIB_ERR_NONE) {
    SPRINT("MAIN:lora_init_fail:" + state);
    while (true) { delay(10); }
  }
  SPRINT("MAIN:lora_init_succ" + state);

  radio.setOutputPower(10.0);
  radio.setCurrentLimit(100.0);
  radio.setDataShaping(RADIOLIB_SHAPING_1_0);
  uint8_t syncWord[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
  radio.setSyncWord(syncWord, 8);
  radio.setCRC(2, 0xFFFF, 0x8005, false);

  // create i2s speaker
  i2s_config_t i2s_speaker_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 256, // 256 for 1200, 128 for 2400 an 3200 // don't use 700C
    .use_apll=0,
    .tx_desc_auto_clear= true, 
    .fixed_mclk=-1    
  };
  i2s_pin_config_t i2s_speaker_pin_config = {
    .bck_io_num = AUDIO_SPEAKER_BCLK,
    .ws_io_num = AUDIO_SPEAKER_LRC,
    .data_out_num = AUDIO_SPEAKER_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  if (i2s_driver_install(I2S_NUM_0, &i2s_speaker_config, 0, NULL) != ESP_OK) {
    SPRINT("MAIN:i2s_spk_install_fail");
  }
  if (i2s_set_pin(I2S_NUM_0, &i2s_speaker_pin_config) != ESP_OK) {
    SPRINT("MAIN:i2s_spk_pins_fail");
  }

  // create i2s microphone
  i2s_config_t i2s_mic_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 512,
    .use_apll=0,
    .tx_desc_auto_clear= true,
    .fixed_mclk=-1
  };
  i2s_pin_config_t i2s_mic_pin_config = {
    .bck_io_num = AUDIO_MIC_SCK,
    .ws_io_num = AUDIO_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = AUDIO_MIC_SD
  };
  if (i2s_driver_install(I2S_NUM_1, &i2s_mic_config, 0, NULL) != ESP_OK) {
    SPRINT("MAIN:i2s_mic_install_fail");
  }
  if (i2s_set_pin(I2S_NUM_1, &i2s_mic_pin_config) != ESP_OK) {
    SPRINT("MAIN:i2s_mic_pins_fail");
  }

  // run audio loopback on a separate task
  xTaskCreate(&audioTask,     "AudioTask",    16384*2, NULL, 1, NULL);
  xTaskCreate(&transmitTask,  "transmitTask", 16384*2, NULL, 1, NULL);

  SPRINT("MAIN:setup_done");
}

void loop() {
  delay(1000);
}