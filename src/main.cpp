#include <config.h>

volatile char radioAction = 0; // 0 - listening, 1 - receiving, 2 - transmitting

void setup()
{
	LOG_SET_LEVEL(DebugLogLevel::LVL_INFO);

	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;

	delay(1000); // wait for platformio's serialmon window change

	LOG_INFO("Board setup started");

	setup_preferences();

	// setup ptt button
	pinMode(PTTBTN_PIN, INPUT);

	int state = radio.begin(LORA_RADIO_FREQ, LORA_RADIO_BW, LORA_RADIO_SF, LORA_RADIO_CR, LORA_RADIO_SYNC, LORA_RADIO_PWR, LORA_RADIO_PL, 0, false);
	if (state != RADIOLIB_ERR_NONE)
	{
		LOG_ERROR("Lora radio start failed:", state);
		for (;;)
		{
			delay(100);
		};
	}

	LOG_INFO("Lora radio initialized");
	radio.setCRC(LORA_RADIO_CRC);
	radio.setRfSwitchPins(LORA_RADIO_PIN_RXEN, LORA_RADIO_PIN_TXEN);
	radio.clearDio1Action();
	radio.setDio1Action(onLoraDataAvailableIsr);
	// radio.setRxBoostedGainMode(true, true);
#ifdef LORA_RADIO_EXPL
	LOG_INFO("Using explicit header");
	radio.explicitHeader();
#else
	LOG_INFO("Using implicit header");
	radio.implicitHeader();
#endif

#ifdef ENABLE_DISPLAY
	setupDisplay();
	xTaskCreate(&displayTask, "displayTask", 8000, NULL, 5, &displayTaskHandle);
#endif
	setupAudio();
	xTaskCreate(&audioTask, "audioTask", 32000, NULL, 5, &audioTaskHandle);

	setupEncoder();
	xTaskCreate(&encoderTask, "audioTask", 32000, NULL, 5, &encoderTaskHandle);

	xTaskCreate(&loraTask, "loraTask", 8000, NULL, 5, &loraTaskHandle);

	state = radio.startReceive();
	if (state != RADIOLIB_ERR_NONE)
	{
		LOG_ERROR("Receive start error:", state);
	}
	LOG_INFO("Board setup completed");

#ifdef ENABLE_LIGHT_SLEEP
	LOG_INFO("Light sleep is enabled");
#endif
	light_sleep_reset();
}

void loop()
{
	bool ptt_state = (analogRead(PTTBTN_PIN) >= 3000);

	if (ptt_state && !ptt_pressed)
	{
		LOG_INFO("PTT pushed, start TX");
		ptt_pressed = true;

		// notify to start recording
		xTaskNotify(audioTaskHandle, AUDIO_TASK_RECORD_BIT, eSetBits);
	}
	else if (!ptt_state && ptt_pressed)
	{
		LOG_INFO("PTT released");
		ptt_pressed = false;
	}
	light_sleep_timer.tick();
	delay(50);
}