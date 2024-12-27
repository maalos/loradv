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

/*
	setup_preferences();

	// setup ptt button
	pinMode(PTTBTN_PIN, INPUT);

	// setup the encoder
	pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT);
	pinMode(ROTARY_ENCODER_A_PIN, INPUT);
	pinMode(ROTARY_ENCODER_B_PIN, INPUT);

	state = radio.begin(getFrequency(), getBandwidth(), getSpreadingFactor(), getCodingRate(), getSyncWord(), getOutputPower(), getPreambleLength(), 0, false);
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

*/
	setupDisplay();

	connectToWiFi();

	xTaskCreate(&displayTask, "displayTask", 64000, NULL, 5, &displayTaskHandle);

	/*
	setupAudio();
	xTaskCreate(&audioTask, "audioTask", 32000, NULL, 5, &audioTaskHandle);	// TODO: lower the stack depth

	setupEncoder();
	xTaskCreate(&encoderTask, "encoderTask", 32000, NULL, 5, &encoderTaskHandle);	// TODO: lower the stack depth

	xTaskCreate(&loraTask, "loraTask", 8000, NULL, 5, &loraTaskHandle);
	*/

	LOG_INFO("Board setup completed");

#ifdef ENABLE_SLEEP
	LOG_INFO("Sleep is enabled");
#endif
	sleepReset();
}

void loop()
{
	bool pttState = (analogRead(PTTBTN_PIN) >= 3000);

	if (pttState && !pttPressed)
	{
		LOG_INFO("PTT pushed, start TX");
		pttPressed = true;

		// notify to start recording
		xTaskNotify(audioTaskHandle, AUDIO_TASK_RECORD_BIT, eSetBits);
	}
	else if (!pttState && pttPressed)
	{
		LOG_INFO("PTT released");
		pttPressed = false;
	}
	sleepTimer.tick();
	delay(50);
}