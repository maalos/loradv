#include <config.h>

volatile char radioAction = 0; // 0 - listening, 1 - receiving, 2 - transmitting

void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;

	// delay(1000); // wait for platformio's serialmon window change // actually, i don't care

	Serial.println(F("Board setup started"));

	setupPreferences();

	// setup ptt button
	pinMode(PTTBTN_PIN, INPUT_PULLDOWN);
	pinMode(LORA_RADIO_PIN_B, INPUT_PULLDOWN);

	// state = radio.begin(getFrequency(), getBandwidth(), getSpreadingFactor(), getCodingRate(), getSyncWord(), getOutputPower(), getPreambleLength(), 0, false);
	state = radio.begin(LORA_RADIO_FREQ, LORA_RADIO_BW, LORA_RADIO_SF, LORA_RADIO_CR, LORA_RADIO_SYNC, LORA_RADIO_PWR, LORA_RADIO_PL, 0, false);
	//Serial.printf("%f, %f, %f, %f, %f, %f, %f", getFrequency(), getBandwidth(), getSpreadingFactor(), getCodingRate(), getSyncWord(), getOutputPower(), getPreambleLength());
	if (state != RADIOLIB_ERR_NONE)
	{
		Serial.printf("Lora radio start failed: %d\n", state);
		for (;;)
		{
			delay(100);
		};
	}

	Serial.println(F("Lora radio initialized\n"));
	radio.setCRC(LORA_RADIO_CRC);
	radio.setRfSwitchPins(LORA_RADIO_PIN_RXEN, LORA_RADIO_PIN_TXEN);
	radio.clearDio1Action();
	radio.setDio1Action(onLoraDataAvailableIsr);
	// radio.setRxBoostedGainMode(true, true);
#ifdef LORA_RADIO_EXPL
	Serial.println(F("Using explicit header"));
	radio.explicitHeader();
#else
	Serial.println(F("Using implicit header"));
	radio.implicitHeader();
#endif
	setupDisplay();

	//connectToWiFi();

	// xTaskCreatePinnedToCore(&displayTask, "displayTask", 6000, NULL, 0, &displayTaskHandle, 0);

	setupAudio();
	xTaskCreatePinnedToCore(&audioTask, "audioTask", 32000, NULL, 10, &audioTaskHandle, 0);	// TODO: lower the stack depth

	// xTaskCreatePinnedToCore(&monitorTask, "monitorTask", 2048, NULL, 5, &monitorTaskHandle, 0);


	// setupEncoder();

	xTaskCreatePinnedToCore(&loraTask, "loraTask", 8000, NULL, 10, &loraTaskHandle, 1);
	Serial.println(F("Board setup completed"));

#ifdef ENABLE_SLEEP
	Serial.println(F("Sleep is enabled"));
	// Create a one-shot FreeRTOS timer
    sleepTimer = xTimerCreate(
        "SleepTimer",                         // name
        pdMS_TO_TICKS(SLEEP_DELAY_MS),        // period
        pdFALSE,                              // one-shot
        (void*)0,                             // ID
        sleepCallback                         // callback
    );
    if (sleepTimer == NULL) {
        Serial.println("Failed to create sleep timer!");
    }

#endif
	sleepReset("Board setup completed");
}

void loop()
{
	bool pttState = (analogRead(PTTBTN_PIN) >= 4000);

	if (pttState && !pttPressed)
	{
		Serial.println(F("PTT pushed, start TX"));
		pttPressed = true;

		resetAGC();

		// notify to start recording
		xTaskNotify(audioTaskHandle, AUDIO_TASK_RECORD_BIT, eSetBits);
	}
	else if (!pttState && pttPressed)
	{
		Serial.println(F("PTT released"));
		pttPressed = false;
	}
	// sleepTimer.tick();
	// encoderTask();
	delay(50);
}