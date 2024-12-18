#include "config.h"

// tasks.cpp
extern void sprint(const char* format, ...);

void setupAudio() {
  sprint("SA:start");
    // create i2s speaker
  i2s_config_t i2s_speaker_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_MSB,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 256, // 256 for 1200, 128 for 2400 an 3200 // don't use 700C
    .use_apll = 0,
    .tx_desc_auto_clear = true, 
    .fixed_mclk = -1    
  };
  i2s_pin_config_t i2s_speaker_pin_config = {
    .bck_io_num = AUDIO_SPEAKER_BCLK,
    .ws_io_num = AUDIO_SPEAKER_LRC,
    .data_out_num = AUDIO_SPEAKER_DIN,
    .data_in_num = I2S_PIN_NO_CHANGE
  };
  if (i2s_driver_install(I2S_NUM_0, &i2s_speaker_config, 0, NULL) != ESP_OK) {
    sprint("SA:i2s_spk_install_fail");
  }
  if (i2s_set_pin(I2S_NUM_0, &i2s_speaker_pin_config) != ESP_OK) {
    sprint("SA:i2s_spk_pins_fail");
  }

  // create i2s microphone
  i2s_config_t i2s_mic_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = AUDIO_SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 2,
    .dma_buf_len = 512,
    .use_apll = 0,
    .tx_desc_auto_clear = true,
    .fixed_mclk = -1
  };
  i2s_pin_config_t i2s_mic_pin_config = {
    .bck_io_num = AUDIO_MIC_SCK,
    .ws_io_num = AUDIO_MIC_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = AUDIO_MIC_SD
  };
  if (i2s_driver_install(I2S_NUM_1, &i2s_mic_config, 0, NULL) != ESP_OK) {
    sprint("SA:i2s_mic_install_fail");
  }
  if (i2s_set_pin(I2S_NUM_1, &i2s_mic_pin_config) != ESP_OK) {
    sprint("SA:i2s_mic_pins_fail");
  }
  sprint("SA:done");
}