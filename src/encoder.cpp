#include <config.h>

#define ROTARY_ENCODER_STEPS 4

AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, -1, ROTARY_ENCODER_STEPS);
TaskHandle_t encoderTaskHandle;

void setupEncoder()
{
    rotaryEncoder.begin();
	Serial.println(F("Encoder setup"));
}

void encoderTask(void *param)
{
	Serial.println(F("Encoder task started"));
    while (true)
    {
        if (rotaryEncoder.encoderChanged())
            Serial.println(rotaryEncoder.readEncoder());
        vTaskDelay(1);
    }
}

/*
CONTROL         ACTION          MENU ACTION
turn left (ccw) volume down     next position
turn right (cw) volume up       previous pos.
short press     enter menu      select
*/
