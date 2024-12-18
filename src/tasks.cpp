#include "config.h"

struct CODEC2* c2_;
int c2_samples_per_frame_;
int c2_bytes_per_frame_;
int16_t* c2_samples_;
uint8_t* c2_bits_;

extern SX1262 radio;

CircularBuffer<uint8_t, BUFFER_SIZE> txQueue;

int state = RADIOLIB_ERR_NONE;

volatile bool transmittedFlag = true;
volatile bool receivedFlag = false;


ICACHE_RAM_ATTR
void setTxFlag(void) {
  transmittedFlag = true;
}

ICACHE_RAM_ATTR
void setRxFlag(void) {
  receivedFlag = true;
}

void sprint(const char* format, ...) {
  va_list args;
  va_start(args, format);
  char buffer[128];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  Serial.println(buffer);
}

void audioTask(void* param) {
  sprint("AT:task_start");

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
    i2s_read(I2S_NUM_1, c2_samples_, sizeof(uint16_t) * c2_samples_per_frame_, &bytes_read, portMAX_DELAY);
    codec2_encode(c2_, c2_bits_, c2_samples_);

    if (!txQueue.push(*c2_bits_)) { // KISS
      sprint("AT:fb_drop");
    }

    vPortYield();

    codec2_decode(c2_, c2_samples_, c2_bits_);
    i2s_write(I2S_NUM_0, c2_samples_, sizeof(uint16_t) * c2_samples_per_frame_, &bytes_written, portMAX_DELAY);
  }
}

void transmitTask(void* param) {
  sprint("TT:task_start");

  radio.setPacketSentAction(setTxFlag);

  String result;
  while (true) {    
    if (!txQueue.isEmpty() && transmittedFlag) {
      result.clear();
      transmittedFlag = false;
      radio.finishTransmit();
      if (state != RADIOLIB_ERR_NONE) {
          sprint("TT:tx_fail:%d", state);
      } else {
        sprint("TT:tx_succ");
      }

      uint8_t queueSize = txQueue.size();
      sprint("TT:DEBUG:txQs:%d", queueSize);

      // set size to the nearest (floor) divisible-by-6 number
      int processSize = queueSize - (queueSize % 6);
      sprint("TT:DEBUG:shifting:%d", processSize);

      for (int i = 0; i < processSize; ++i) {
        result += (char)txQueue.shift();
      }

      //sprint("TT:DEBUG:tx:" + result);
      state = radio.startTransmit(result); // max 256 bytes acc. to LLCC68's datasheet
    }

    vPortYield();
  }
}

void receiveTask(void* param) { // should be killed when PTT is pressed
  sprint("RT:task_start");

  radio.setPacketReceivedAction(setRxFlag);
  state = radio.startReceive();
  if (state == RADIOLIB_ERR_NONE) {
    sprint("RT:init:succ");
  } else {
    sprint("RT:init:fail:%d", state);
  }

  while(true) {
    if(receivedFlag) {
      receivedFlag = false;

      String str;
      int state = radio.readData(str);

      if (state == RADIOLIB_ERR_NONE) {
        // now we should split the string into frames
        // of 6-8 bytes depending on C2's speed, then
        // add them to a queue/circularbuffer
        sprint("RT:DEBUG:recv:%s", str);
        
        sprint("RT:recv:succ|RSSI:%fdBm|SNR:%fdB|FE:%fHz",
              radio.getRSSI(),radio.getSNR(), radio.getFrequencyError()); // idk if this shouldn't be before the if statement

      } else if (state == RADIOLIB_ERR_CRC_MISMATCH) {
        sprint("RT:recv:fail:crc");
      } else {
        sprint("RT:recv:fail:%d", state);
      }
    }
  }
}