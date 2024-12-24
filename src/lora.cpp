#include <config.h>

TaskHandle_t loraTaskHandle;
SX1262 radio = new Module(LORA_RADIO_PIN_SS, LORA_RADIO_PIN_A, LORA_RADIO_PIN_RST, LORA_RADIO_PIN_B);

// lora task packet and packet index/size queues
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_rx_queue;
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_rx_queue_index;
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_tx_queue;
CircularBuffer<uint8_t, LORA_RADIO_QUEUE_LEN> lora_radio_tx_queue_index;

// packet buffers
byte lora_radio_rx_buf[LORA_RADIO_BUF_LEN]; // tx packet buffer
byte lora_radio_tx_buf[LORA_RADIO_BUF_LEN]; // rx packet buffer

volatile bool lora_enable_isr = true; // true to enable rx isr, disabled on tx
volatile bool ptt_pressed = false;

unsigned long lastI2STime = 0;

// lora trasmit receive task
void loraTask(void *param)
{
	LOG_INFO("Lora task started");

	// wait for ISR notification, read data and send for audio processing
	while (true)
	{
		uint32_t lora_status_bits = 0;
		xTaskNotifyWaitIndexed(0, 0x00, ULONG_MAX, &lora_status_bits, portMAX_DELAY);

		LOG_DEBUG("Lora task bits", lora_status_bits);

		// lora rx
		if (lora_status_bits & LORA_RADIO_TASK_RX_BIT)
		{
			int packet_size = radio.getPacketLength();
			if (packet_size <= 0)
			{
				continue;
			}

			int state = radio.readData(lora_radio_rx_buf, packet_size);
			if (state != RADIOLIB_ERR_NONE)
			{
				LOG_ERROR("Read data error: ", state);
			}

			// process packet
			LOG_DEBUG("Received packet, size", packet_size);
			if (packet_size % c2_bytes_per_frame != 0)
			{
				LOG_ERROR("Audio packet of wrong size, expected mod", c2_bytes_per_frame);
				continue;
			}

			for (int i = 0; i < packet_size; i++)
			{
				lora_radio_rx_queue.push(lora_radio_rx_buf[i]);
			}

			lora_radio_rx_queue_index.push(packet_size);
			xTaskNotify(audioTaskHandle, AUDIO_TASK_PLAY_BIT, eSetBits);

			// probably not needed, still in receive
			state = radio.startReceive();
			if (state != RADIOLIB_ERR_NONE)
			{
				LOG_ERROR("Start receive error: ", state);
			}
			sleepReset();
		} // lora rx
		// lora tx data
		else if (lora_status_bits & LORA_RADIO_TASK_TX_BIT)
		{
			lora_enable_isr = false;
			// take packet by packet
			while (!lora_radio_tx_queue_index.isEmpty())
			{
				radioAction = 2;
				// take packet size and read it
				int tx_bytes_cnt = lora_radio_tx_queue_index.shift();
				for (int i = 0; i < tx_bytes_cnt; i++)
				{
					lora_radio_tx_buf[i] = lora_radio_tx_queue.shift();
				}

				// transmit packet
				int state = radio.transmit(lora_radio_tx_buf, tx_bytes_cnt);
				if (state != RADIOLIB_ERR_NONE)
				{
					LOG_ERROR("Lora radio transmit failed:", state);
					continue;
				}
				sleepReset();

				LOG_DEBUG("Transmitted packet", tx_bytes_cnt);
				vTaskDelay(1);
			} // packet transmit loop
			radioAction = 0;

			// switch to receive after all transmitted
			int state = radio.startReceive();
			if (state != RADIOLIB_ERR_NONE)
			{
				LOG_ERROR("Start receive error: ", state);
			}
			lora_enable_isr = true;
			sleepReset();
		} // lora tx
	} // task loop
}

// ISR is called when new data is available from radio
ICACHE_RAM_ATTR void onLoraDataAvailableIsr()
{
	if (!lora_enable_isr)
		return;
	BaseType_t xHigherPriorityTaskWoken;
	radioAction = 1;
	// notify radio receive task on new data arrival
	xTaskNotifyFromISR(loraTaskHandle, LORA_RADIO_TASK_RX_BIT, eSetBits, &xHigherPriorityTaskWoken);
}

float getFrequency()
{
	return getSetting("FQ");
}

void setFrequency(float freq)
{
	setSetting("FQ", freq);
	radio.setFrequency(freq);
}

float getBandwidth()
{
	return getSetting("BW");
}

void setBandwidth(float bw)
{
	setSetting("BW", bw);
	radio.setBandwidth(bw);
}

float getSpreadingFactor()
{
	return getSetting("SF");
}

void setSpreadingFactor(float sf)
{
	setSetting("SF", sf);
	radio.setSpreadingFactor(sf);
}

float getCodingRate()
{
	return getSetting("CR");
}

void setCodingRate(float cr)
{
	setSetting("CR", cr);
	radio.setCodingRate(cr);
}

float getSyncWord()
{
	return getSetting("SW");
}

void setSyncWord(float sw)
{
	setSetting("SW", sw);
	radio.setSyncWord(sw);
}

float getOutputPower()
{
	return getSetting("TXP");
}

void setOutputPower(float txp)
{
	setSetting("TXP", txp);
	radio.setOutputPower(txp);
}

float getPreambleLength()
{
	return getSetting("PL");
}

void setPreambleLength(float pl)
{
	setSetting("PL", pl);
	radio.setPreambleLength(pl);
}

float getCrcLength()
{
	return getSetting("CRC");
}

void setCrcLength(float crcl)
{
	setSetting("CRC", crcl);
	radio.setCRC(crcl);
}
