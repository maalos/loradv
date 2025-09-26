#include <config.h>

TaskHandle_t audioTaskHandle;
// codec2
struct CODEC2 *c2;		  // codec2 instance
int c2_samples_per_frame; // how many raw samples in one frame
int c2_bytes_per_frame;	  // how many bytes in encoded frame
int16_t *c2_samples;	  // buffer for raw samples
uint8_t *c2_bits;		  // buffer for encoded frame
float volume = 5;

void adjustGain(int16_t* pcmBuffer, int pcmBufferSize, float gain) {
  for (int i = 0; i < pcmBufferSize; i++)
    pcmBuffer[i] *= gain;
}

void setupAudio()
{
	// setup speaker
	i2s_config_t i2s_speaker_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
		.sample_rate = AUDIO_SAMPLE_RATE,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_STAND_MSB,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = 2 * 4, // 8
		.dma_buf_len = 192 * 4, // 512
		.use_apll = 0,
		.tx_desc_auto_clear = true,
		.fixed_mclk = -1};
	i2s_pin_config_t i2s_speaker_pin_config = {
		.bck_io_num = AUDIO_I2S_SCK,
		.ws_io_num = AUDIO_SPEAKER_LRC,
		.data_out_num = AUDIO_SPEAKER_DIN,
		.data_in_num = I2S_PIN_NO_CHANGE};
	if (i2s_driver_install(I2S_NUM_0, &i2s_speaker_config, 0, NULL) != ESP_OK)
	{
		Serial.println(F("Failed to install i2s speaker driver"));
	}
	if (i2s_set_pin(I2S_NUM_0, &i2s_speaker_pin_config) != ESP_OK)
	{
		Serial.println(F("Failed to set i2s speaker pins"));
	}

	// setup microphone
	i2s_config_t i2s_mic_config = {
		.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
		.sample_rate = AUDIO_SAMPLE_RATE,
		.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
		.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
		.communication_format = I2S_COMM_FORMAT_STAND_I2S,
		.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
		.dma_buf_count = 2 * 4, // 8
		.dma_buf_len = 64 * 4, // 256
		.use_apll = 0,
		.tx_desc_auto_clear = true,
		.fixed_mclk = -1};
	i2s_pin_config_t i2s_mic_pin_config = {
		.bck_io_num = AUDIO_I2S_SCK,
		.ws_io_num = AUDIO_MIC_WS,
		.data_out_num = I2S_PIN_NO_CHANGE,
		.data_in_num = AUDIO_MIC_SD};
	if (i2s_driver_install(I2S_NUM_1, &i2s_mic_config, 0, NULL) != ESP_OK)
	{
		Serial.println(F("Failed to install i2s mic driver"));
	}
	if (i2s_set_pin(I2S_NUM_1, &i2s_mic_pin_config) != ESP_OK)
	{
		Serial.println(F("Failed to set i2s mic pins"));
	}
}

#if CODEC2_MODE == CODEC2_MODE_700C
#define AUDIO_MAX_PACKET_SIZE 48
#elif CODEC2_MODE == CODEC2_MODE_1200 || CODEC2_MODE == CODEC2_MODE_1300 || CODEC2_MODE == CODEC2_MODE_1400 || CODEC2_MODE == CODEC2_MODE_1600 || CODEC2_MODE == CODEC2_MODE_2400
#define AUDIO_MAX_PACKET_SIZE 96
#elif CODEC2_MODE == CODEC2_MODE_3200
#define AUDIO_MAX_PACKET_SIZE 144
#endif

typedef struct {
    float a0, a1, a2, b1, b2;
    float z1, z2;
} Biquad;

void biquad_init_highpass(Biquad *f, float sample_rate, float cutoff_hz, float q) {
    float w0 = 2.0f * M_PI * cutoff_hz / sample_rate;
    float cosw0 = cosf(w0);
    float sinw0 = sinf(w0);
    float alpha = sinw0 / (2.0f * q);

    float b0 = (1 + cosw0) / 2;
    float b1 = -(1 + cosw0);
    float b2 = (1 + cosw0) / 2;
    float a0 = 1 + alpha;
    float a1 = -2 * cosw0;
    float a2 = 1 - alpha;

    f->a0 = b0 / a0;
    f->a1 = b1 / a0;
    f->a2 = b2 / a0;
    f->b1 = a1 / a0;
    f->b2 = a2 / a0;

    f->z1 = f->z2 = 0;
}

void biquad_init_lowpass(Biquad *f, float sample_rate, float cutoff_hz, float q) {
    float w0 = 2.0f * M_PI * cutoff_hz / sample_rate;
    float cosw0 = cosf(w0);
    float sinw0 = sinf(w0);
    float alpha = sinw0 / (2.0f * q);

    float b0 = (1 - cosw0) / 2;
    float b1 = 1 - cosw0;
    float b2 = (1 - cosw0) / 2;
    float a0 = 1 + alpha;
    float a1 = -2 * cosw0;
    float a2 = 1 - alpha;

    f->a0 = b0 / a0;
    f->a1 = b1 / a0;
    f->a2 = b2 / a0;
    f->b1 = a1 / a0;
    f->b2 = a2 / a0;

    f->z1 = f->z2 = 0;
}

inline int16_t biquad_process(Biquad *f, int16_t in) {
    float out = f->a0 * in + f->a1 * f->z1 + f->a2 * f->z2
                - f->b1 * f->z1 - f->b2 * f->z2;

    f->z2 = f->z1;
    f->z1 = out;

    if (out > 32767) out = 32767;
    else if (out < -32768) out = -32768;

    return (int16_t)out;
}

Biquad hpf_, lpf;

// audio record/playback encode/decode task
void audioTask(void *param)
{
	Serial.println(F("Audio task started"));

	// construct codec2
	c2 = codec2_create(CODEC2_MODE);
	if (c2 == NULL)
	{
		Serial.println(F("Failed to create codec2"));
		for (;;)
		{
			delay(1000);
		}
	}

	// codec2_set_lpc_post_filter(c2, CODEC2_LPC_PF_ENABLE, CODEC2_LPC_PF_BASSBOOST, CODEC2_LPC_PF_BETA, CODEC2_LPC_PF_GAMMA);
	c2_samples_per_frame = codec2_samples_per_frame(c2);
	c2_bytes_per_frame = codec2_bytes_per_frame(c2);
	c2_samples = (int16_t *)malloc(sizeof(int16_t) * c2_samples_per_frame);
	c2_bits = (uint8_t *)malloc(sizeof(uint8_t) * c2_bytes_per_frame);
	Serial.println(F("C2 initialized"));

	biquad_init_highpass(&hpf_, 8000.0f, 80.0f, 0.707f);
  	biquad_init_lowpass(&lpf, 8000.0f, 3000.0f, 0.707f);

	// wait for data notification, decode frames and playback
	size_t bytes_read, bytes_written;
	while (true)
	{
		uint32_t audio_bits = 0;
		xTaskNotifyWaitIndexed(0, 0x00, ULONG_MAX, &audio_bits, portMAX_DELAY);

		// Serial.printf("Audio task bits: %d", audio_bits);

		// audio rx-decode-playback
		if (audio_bits & AUDIO_TASK_PLAY_BIT)
		{
			// Serial.println(F("Playing audio"));
			// while rx frames are available and button is not pressed
			while (!pttPressed && !lora_radio_rx_queue_index.isEmpty())
			{
				int packet_size = lora_radio_rx_queue_index.shift();
				// Serial.println(F("Playing packet"));
				// split by frame, decode and play
				for (int i = 0; i < packet_size; i++)
				{
					c2_bits[i % c2_bytes_per_frame] = lora_radio_rx_queue.shift();
					if (i % c2_bytes_per_frame != c2_bytes_per_frame - 1)
						continue;
					codec2_decode(c2, c2_samples, c2_bits);
					i2s_write(I2S_NUM_0, c2_samples, sizeof(uint16_t) * c2_samples_per_frame, &bytes_written, portMAX_DELAY);
					vTaskDelay(1);
				}
			} // while rx data available
		} // audio decode playback
		// audio record-encode-tx
		else if (audio_bits & AUDIO_TASK_RECORD_BIT)
		{
			//Serial.println(F("Recording audio"));
			int packet_size = 0;
			// record while button is pressed
			while (pttPressed)
			{
				// send packet if enough audio encoded frames are accumulated
				if (packet_size + c2_bytes_per_frame > AUDIO_MAX_PACKET_SIZE)
				{
					//Serial.println(F("Recorded packet"));
					lora_radio_tx_queue_index.push(packet_size);
					xTaskNotify(loraTaskHandle, LORA_RADIO_TASK_TX_BIT, eSetBits);
					packet_size = 0;
				}
				// read and encode one sample
				size_t bytes_read;
				i2s_read(I2S_NUM_1, c2_samples, sizeof(uint16_t) * c2_samples_per_frame, &bytes_read, portMAX_DELAY);

				// adjustGain(c2_samples, c2_samples_per_frame, 5);

    			for (int i = 0; i < c2_samples_per_frame; i++) {
				    int16_t s = biquad_process(&hpf_, c2_samples[i]);
				    c2_samples[i] = biquad_process(&lpf, s);
				}

				codec2_encode(c2, c2_bits, c2_samples);
				for (int i = 0; i < c2_bytes_per_frame; i++)
				{
					lora_radio_tx_queue.push(c2_bits[i]);
				}
				packet_size += c2_bytes_per_frame;
				vTaskDelay(1);
			} // pttPressed
			// send remaining tail audio encoded samples
			if (packet_size > 0)
			{
				//Serial.println(F("Recorded packet"));
				lora_radio_tx_queue_index.push(packet_size);
				xTaskNotify(loraTaskHandle, LORA_RADIO_TASK_TX_BIT, eSetBits);
				packet_size = 0;
			}
			vTaskDelay(1);
		} // task bit
	}
}

TaskHandle_t monitorTaskHandle = NULL;

// Stack monitor task
void monitorTask(void *pvParameters) {
    while (true) {
        if (audioTaskHandle != NULL) {
            UBaseType_t watermark = uxTaskGetStackHighWaterMark(audioTaskHandle);
            Serial.printf(
                "[Stack Monitor] AudioTask min remaining stack: %u words (%u bytes)\n",
                watermark,
                watermark * 4
            );
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // print every 2s
    }
}
