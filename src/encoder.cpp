#include <config.h>

#define ROTARY_ENCODER_STEPS 4

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile long encoder0Pos = 0;
long encoderSteps = 4;
long encoderMinValue = 0;
long encoderMaxValue = 25;
long currentPos;

void IRAM_ATTR readEncoderISR()
{
    static int8_t old_AB = 0;
    static int8_t enc_states[16] = {0, 1, -1, 0, -1, 0, 0, 1, 1, 0, 0, -1, 0, -1, 1, 0}; // cw is more
//  static int8_t enc_states[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0}; // cw is less


    old_AB <<= 2;
    int8_t ENC_PORT = ((digitalRead(ROTARY_ENCODER_B_PIN)) ? (1 << 1) : 0) | ((digitalRead(ROTARY_ENCODER_A_PIN)) ? (1 << 0) : 0);
    old_AB |= (ENC_PORT & 0x03);

    portENTER_CRITICAL_ISR(&mux);
    encoder0Pos += enc_states[(old_AB & 0x0f)];

    if (encoder0Pos / encoderSteps < encoderMinValue)
    {
        encoder0Pos = encoderMinValue * encoderSteps;
    }
    else if (encoder0Pos / encoderSteps > encoderMaxValue)
    {
        encoder0Pos = encoderMaxValue * encoderSteps;
    }

    portEXIT_CRITICAL_ISR(&mux);
}

void setupEncoder()
{
    pinMode(ROTARY_ENCODER_A_PIN, INPUT);
    pinMode(ROTARY_ENCODER_B_PIN, INPUT);
    pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT);
    
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN), readEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN), readEncoderISR, CHANGE);
    Serial.println(F("Encoder setup"));
}

void encoderTask()
{
    static long lastReadEncoder0Pos = 0;
    currentPos = encoder0Pos / encoderSteps;

    if (currentPos != lastReadEncoder0Pos)
    {
        Serial.printf("Volume: %ld%%\n", currentPos * 4);
        lastReadEncoder0Pos = currentPos;
    }

    if (analogRead(ROTARY_ENCODER_BUTTON_PIN) == 4095)
    {
        Serial.println("Button pressed");
    }
}
