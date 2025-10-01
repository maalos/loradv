#include <config.h>

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile long encoder0Pos = 10;
long encoderSteps = 1;
long encoderMinValue = 0;
long encoderMaxValue = 100;
volatile bool encoderButtonPressed = false;

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

    if (encoder0Pos < encoderMinValue * encoderSteps) encoder0Pos = encoderMinValue * encoderSteps;
    if (encoder0Pos > encoderMaxValue * encoderSteps) encoder0Pos = encoderMaxValue * encoderSteps;

    Serial.printf("Volume: %ld%%\n", encoder0Pos);

    portEXIT_CRITICAL_ISR(&mux);
}


void IRAM_ATTR readButtonISR()
{
    Serial.println(F("Encoder button down"));
    encoderButtonPressed = digitalRead(ROTARY_ENCODER_BUTTON_PIN);
}

void setupEncoder()
{
    pinMode(ROTARY_ENCODER_A_PIN, INPUT_PULLDOWN);
    pinMode(ROTARY_ENCODER_B_PIN, INPUT_PULLDOWN);
    pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLDOWN);
    
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN), readEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN), readEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_BUTTON_PIN), readButtonISR, CHANGE);
    Serial.println(F("Encoder setup"));
}