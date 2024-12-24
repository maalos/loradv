#include <config.h>

Timer<1> sleepTimer;
Timer<1>::Task sleepTimerTask;

// called by timer to enter sleep
bool sleep(void *param)
{
#ifdef ENABLE_SLEEP
	LOG_INFO("Entering sleep");
	radioAction = 0;
	vTaskDelay(2000);
	radioAction = 0;
	// wake up on ptt button or lora radio incoming data
	digitalWrite(TFT_BL, LOW);
	esp_sleep_enable_ext1_wakeup(SLEEP_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
	delay(100);
	esp_light_sleep_start();
	sleepReset();
	LOG_INFO("Exiting sleep");
	digitalWrite(TFT_BL, HIGH);
#endif
	return false;
}

// start counting timer for sleep
void sleepReset()
{
#ifdef ENABLE_SLEEP
	LOG_INFO("Reset sleep");
	if (sleepTimerTask != NULL)
		sleepTimer.cancel(sleepTimerTask);
	sleepTimerTask = sleepTimer.in(SLEEP_DELAY_MS, sleep);
#endif
}