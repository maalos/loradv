#include <config.h>

// codec2 
struct CODEC2* c2;             // codec2 instance
int c2_samples_per_frame;      // how many raw samples in one frame
int c2_bytes_per_frame;        // how many bytes in encoded frame
int16_t *c2_samples;           // buffer for raw samples
uint8_t *c2_bits;              // buffer for encoded frame

// lora task packet and packet index/size queues
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_rx_queue;
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_rx_queue_index;
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_tx_queue;
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_tx_queue_index;

// packet buffers
byte lora_radio_rx_buf[LORA_RADIO_BUF_LEN];  // tx packet buffer
byte lora_radio_tx_buf_[LORA_RADIO_BUF_LEN];  // rx packet buffer

volatile bool lora_enable_isr_ = true;  // true to enable rx isr, disabled on tx
volatile bool ptt_pressed = false;

extern Timer<1> light_sleep_timer;
extern bool light_sleep(void *param);
extern void light_sleep_reset();

extern TaskHandle_t audio_task_handle;
extern TaskHandle_t lora_task_handle;

extern SX1262 radio;

#if   CODEC2_MODE == CODEC2_MODE_700C
    #define AUDIO_MAX_PACKET_SIZE 48
#elif CODEC2_MODE == CODEC2_MODE_1200 || CODEC2_MODE == CODEC2_MODE_1300 || CODEC2_MODE == CODEC2_MODE_1400 || CODEC2_MODE == CODEC2_MODE_1600 || CODEC2_MODE == CODEC2_MODE_2400
    #define AUDIO_MAX_PACKET_SIZE 96
#elif CODEC2_MODE == CODEC2_MODE_3200
    #define AUDIO_MAX_PACKET_SIZE 144
#endif

// audio record/playback encode/decode task
void audio_task(void *param) {
  LOG_INFO("Audio task started");

  // construct codec2
  c2 = codec2_create(CODEC2_MODE);
  if (c2 == NULL) {
    LOG_ERROR("Failed to create codec2");
    for(;;) { delay(1000); }
  }

  codec2_set_lpc_post_filter(c2, CODEC2_LPC_PF_ENABLE, CODEC2_LPC_PF_BASSBOOST, CODEC2_LPC_PF_BETA, CODEC2_LPC_PF_GAMMA);
  c2_samples_per_frame = codec2_samples_per_frame(c2);
  c2_bytes_per_frame = codec2_bytes_per_frame(c2);
  c2_samples = (int16_t*)malloc(sizeof(int16_t) * c2_samples_per_frame);
  c2_bits = (uint8_t*)malloc(sizeof(uint8_t) * c2_bytes_per_frame);
  LOG_INFO("C2 initialized", c2_samples_per_frame, c2_bytes_per_frame);

  // wait for data notification, decode frames and playback
  size_t bytes_read, bytes_written;
  while(true) {
    uint32_t audio_bits = 0;
    xTaskNotifyWaitIndexed(0, 0x00, ULONG_MAX, &audio_bits, portMAX_DELAY);

    LOG_DEBUG("Audio task bits", audio_bits);

    // audio rx-decode-playback
    if (audio_bits & AUDIO_TASK_PLAY_BIT) {
      LOG_INFO("Playing audio");
      // while rx frames are available and button is not pressed
      while (!ptt_pressed && !lora_radio_rx_queue_index.isEmpty()) {
        int packet_size = lora_radio_rx_queue_index.shift();
        LOG_INFO("Playing packet", packet_size);
        // split by frame, decode and play
        for (int i = 0; i < packet_size; i++) {
          c2_bits[i % c2_bytes_per_frame] = lora_radio_rx_queue.shift();
          if (i % c2_bytes_per_frame != c2_bytes_per_frame - 1) continue;
          codec2_decode(c2, c2_samples, c2_bits);
          i2s_write(I2S_NUM_0, c2_samples, sizeof(uint16_t) * c2_samples_per_frame, &bytes_written, portMAX_DELAY);
          vTaskDelay(1);
        }
      } // while rx data available
    } // audio decode playback
    // audio record-encode-tx
    else if (audio_bits & AUDIO_TASK_RECORD_BIT) {
      LOG_INFO("Recording audio");
      int packet_size = 0;
      // record while button is pressed
      while (ptt_pressed) {
        // send packet if enough audio encoded frames are accumulated
        if (packet_size + c2_bytes_per_frame > AUDIO_MAX_PACKET_SIZE) {
          LOG_DEBUG("Recorded packet", packet_size);
          lora_radio_tx_queue_index.push(packet_size);
          xTaskNotify(lora_task_handle, LORA_RADIO_TASK_TX_BIT, eSetBits);
          packet_size = 0;
        }
        // read and encode one sample
        size_t bytes_read;
        i2s_read(I2S_NUM_1, c2_samples, sizeof(uint16_t) * c2_samples_per_frame, &bytes_read, portMAX_DELAY);
        codec2_encode(c2, c2_bits, c2_samples);
        for (int i = 0; i < c2_bytes_per_frame; i++) {
          lora_radio_tx_queue.push(c2_bits[i]);
        }
        packet_size += c2_bytes_per_frame;
        vTaskDelay(1);
      } // ptt_pressed
      // send remaining tail audio encoded samples
      if (packet_size > 0) {
          LOG_INFO("Recorded packet", packet_size);
          lora_radio_tx_queue_index.push(packet_size);
          xTaskNotify(lora_task_handle, LORA_RADIO_TASK_TX_BIT, eSetBits);        
          packet_size = 0;
      }
      vTaskDelay(1);
    } // task bit
  }
}

// lora trasmit receive task
void lora_task(void *param) {
  LOG_INFO("Lora task started");

  // wait for ISR notification, read data and send for audio processing
  while (true) {
    uint32_t lora_status_bits = 0;
    xTaskNotifyWaitIndexed(0, 0x00, ULONG_MAX, &lora_status_bits, portMAX_DELAY);

    LOG_DEBUG("Lora task bits", lora_status_bits);

    // lora rx
    if (lora_status_bits & LORA_RADIO_TASK_RX_BIT) {
      int packet_size = radio.getPacketLength();
      if (packet_size <= 0) { continue; }

      int state = radio.readData(lora_radio_rx_buf, packet_size);
      if (state != RADIOLIB_ERR_NONE) {
        LOG_ERROR("Read data error: ", state);
      }

      // process packet
      LOG_INFO("Received packet, size", packet_size);
      if (packet_size % c2_bytes_per_frame != 0) {
        LOG_ERROR("Audio packet of wrong size, expected mod", c2_bytes_per_frame);
        continue;
      }

      for (int i = 0; i < packet_size; i++) {
        lora_radio_rx_queue.push(lora_radio_rx_buf[i]);
      }

      lora_radio_rx_queue_index.push(packet_size);
      xTaskNotify(audio_task_handle, AUDIO_TASK_PLAY_BIT, eSetBits);

      // probably not needed, still in receive
      state = radio.startReceive();
      if (state != RADIOLIB_ERR_NONE) {
        LOG_ERROR("Start receive error: ", state);
      }
      light_sleep_reset();
    } // lora rx
    // lora tx data
    else if (lora_status_bits & LORA_RADIO_TASK_TX_BIT) {
      lora_enable_isr_ = false;
      // take packet by packet
      while (!lora_radio_tx_queue_index.isEmpty()) {
        // take packet size and read it
        int tx_bytes_cnt = lora_radio_tx_queue_index.shift();
        for (int i = 0; i < tx_bytes_cnt; i++) {
          lora_radio_tx_buf_[i] = lora_radio_tx_queue.shift();
        }

        // transmit packet
        int state = radio.transmit(lora_radio_tx_buf_, tx_bytes_cnt);
        if (state != RADIOLIB_ERR_NONE) {
          LOG_ERROR("Lora radio transmit failed:", state);
          continue;
        }

        LOG_DEBUG("Transmitted packet", tx_bytes_cnt);
        vTaskDelay(1);
      } // packet transmit loop
      
      // switch to receive after all transmitted
      int state = radio.startReceive();
      if (state != RADIOLIB_ERR_NONE) {
        LOG_ERROR("Start receive error: ", state);
      }
      lora_enable_isr_ = true;
      light_sleep_reset();
    } // lora tx
  }  // task loop
}

// ISR is called when new data is available from radio
ICACHE_RAM_ATTR void onLoraDataAvailableIsr() {
  if (!lora_enable_isr_) return;
  BaseType_t xHigherPriorityTaskWoken;
  // notify radio receive task on new data arrival
  xTaskNotifyFromISR(lora_task_handle, LORA_RADIO_TASK_RX_BIT, eSetBits, &xHigherPriorityTaskWoken);
}