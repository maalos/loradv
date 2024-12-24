#include <config.h>

Timer<1> light_sleep_timer;			   // light sleep timer
Timer<1>::Task light_sleep_timer_task; // light sleep timer task

// called by timer to enter light sleep
bool light_sleep(void *param)
{
#ifdef ENABLE_LIGHT_SLEEP
	LOG_INFO("Entering light sleep");
	// wake up on ptt button or lora radio incoming data
	esp_sleep_enable_ext0_wakeup(PTTBTN_NUM_PIN, 0);
	esp_sleep_enable_ext1_wakeup(LIGHT_SLEEP_BITMASK, ESP_EXT1_WAKEUP_ANY_HIGH);
	delay(100);
	esp_light_sleep_start();
	LOG_INFO("Exiting light sleep");
#endif
	return false;
}

// start counting timer for light sleep
void light_sleep_reset()
{
#ifdef ENABLE_LIGHT_SLEEP
	LOG_INFO("Reset light sleep");
	if (light_sleep_timer_task != NULL)
		light_sleep_timer.cancel(light_sleep_timer_task);
	light_sleep_timer_task = light_sleep_timer.in(LIGHT_SLEEP_DELAY_MS, light_sleep);
#endif
}