#include "config.h"

// Codec2 variables
struct CODEC2* c2_;
int c2_samples_per_frame_;
int c2_bytes_per_frame_;
int16_t* c2_samples_;
uint8_t* c2_bits_;

extern SX1262 radio;

CircularBuffer<uint8_t, BUFFER_SIZE> txQueue;

void sprint(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[128];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.println(buffer);
}

// Task to encode and decode audio
void audioTask(void* param) {
  sprint("AT:task_start");

  // Construct and configure Codec2
  c2_ = codec2_create(CODEC);
  if (c2_ == NULL) {
    sprint("AT:c2_create_fail");
    return;
  } else {
    codec2_set_lpc_post_filter(c2_, CODEC2_LPC_PF_ENABLE, CODEC2_LPC_PF_BASSBOOST, CODEC2_LPC_PF_BETA, CODEC2_LPC_PF_GAMMA);
    c2_samples_per_frame_ = codec2_samples_per_frame(c2_);
    c2_bytes_per_frame_ = codec2_bytes_per_frame(c2_);
    c2_samples_ = (int16_t*)malloc(sizeof(int16_t) * c2_samples_per_frame_);
    c2_bits_ = (uint8_t*)malloc(sizeof(uint8_t) * c2_bytes_per_frame_);
    sprint("AT:c2_create_succ|%d:%d", c2_samples_per_frame_, c2_bytes_per_frame_);
  }

  size_t bytes_read, bytes_written;
  while (true) {
    // Read audio samples and encode them
    i2s_read(I2S_NUM_1, c2_samples_, sizeof(uint16_t) * c2_samples_per_frame_, &bytes_read, portMAX_DELAY);
    codec2_encode(c2_, c2_bits_, c2_samples_);

    if (!txQueue.push(*c2_bits_)) { // KISS
      sprint("AT:fb_drop");
    }

    vPortYield();

    // Decode samples and play them
    codec2_decode(c2_, c2_samples_, c2_bits_);
    i2s_write(I2S_NUM_0, c2_samples_, sizeof(uint16_t) * c2_samples_per_frame_, &bytes_written, portMAX_DELAY);
  }
}

void transmitTask(void* param) {
  sprint("TT:task_start");
  String result;
  while (true) {    
    if (!txQueue.isEmpty()) {
      result.clear();

      sprint("TT:DEBUG:txQs:%u", txQueue.size());

      for (int i = 0; i < txQueue.size(); ++i) {
        result += (char)txQueue.shift();
      }

      //sprint("TT:DEBUG:tx:" + result);
      int state = radio.transmit(result); // max 256 bytes acc. to LLCC68's datasheet
      if (state != RADIOLIB_ERR_NONE) {
          sprint("TT:tx_fail:%d", state);
      } else {
        sprint("TT:tx_succ");
      }
    }

    vPortYield();
  }
}