#include <config.h>

volatile char radioAction = 0; // 0 - listening, 1 - receiving, 2 - transmitting

void setup()
{
	Serial.begin(SERIAL_BAUD_RATE);
	while (!Serial)
		;

	delay(1000); // wait for platformio's serialmon window change

	Serial.println(F("Board setup started"));

	setupPreferences();

	// setup ptt button
	pinMode(PTTBTN_PIN, INPUT);

	state = radio.begin(getFrequency(), getBandwidth(), getSpreadingFactor(), getCodingRate(), getSyncWord(), getOutputPower(), getPreambleLength(), 0, false);
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
	//setupDisplay();

	//connectToWiFi();

	//xTaskCreatePinnedToCore(&displayTask, "displayTask", 32000, NULL, 0, &displayTaskHandle, 0);

	//setupAudio();
	//xTaskCreatePinnedToCore(&audioTask, "audioTask", 32000, NULL, 10, &audioTaskHandle, 0);	// TODO: lower the stack depth

	setupEncoder();

	//xTaskCreatePinnedToCore(&loraTask, "loraTask", 8000, NULL, 10, &loraTaskHandle, 1);
	//Serial.println(F("Board setup completed"));

#ifdef ENABLE_SLEEP
	Serial.println(F("Sleep is enabled"));
#endif
	sleepReset();
}

void loop()
{

	bool pttState = (analogRead(PTTBTN_PIN) >= 3000);

	if (pttState && !pttPressed)
	{
		Serial.println(F("PTT pushed, start TX"));
		pttPressed = true;

		// notify to start recording
		xTaskNotify(audioTaskHandle, AUDIO_TASK_RECORD_BIT, eSetBits);
	}
	else if (!pttState && pttPressed)
	{
		Serial.println(F("PTT released"));
		pttPressed = false;
	}
	sleepTimer.tick();
	encoderTask();
	delay(50);
}