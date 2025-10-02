#include <config.h>

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
volatile long encoder0Pos = 25;
long encoderSteps = 1;
long encoderMinValue = 0;
long encoderMaxValue = 100;
volatile bool encoderButtonPressed = false;
volatile bool encoderMoved = true;

const int SLEEP_HOLD_TIME = 2000; // 3s
unsigned long buttonPressStart = 0;
bool buttonHeld = false;

void checkButtonHold()
{
    int state = digitalRead(ROTARY_ENCODER_BUTTON_PIN);

    if (state == HIGH) {
        if (buttonPressStart == 0) {
            buttonPressStart = millis();
        }
        if (!buttonHeld && (millis() - buttonPressStart > SLEEP_HOLD_TIME)) {
            buttonHeld = true;
            Serial.println("Going to deep sleep");
            
            radio.sleep(true);

            digitalWrite(DISPLAY_BACKLIGHT_PIN, 0);
            // https://www.reddit.com/r/esp32/comments/movnp3/comment/hlad7wh/
            tft.writecommand(ST7789_DISPOFF);
            delay(100);
            tft.writecommand(ST7789_SLPIN);

            delay(SLEEP_HOLD_TIME); // time to let go off the button

            esp_sleep_enable_ext0_wakeup((gpio_num_t)ROTARY_ENCODER_BUTTON_PIN, 1);
            esp_deep_sleep_start();
        }
    } else {
        buttonPressStart = 0;
        buttonHeld = false;
    }
}


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

    encoderMoved = true;

    portEXIT_CRITICAL_ISR(&mux);
}

void IRAM_ATTR readButtonISR()
{
    static unsigned long lastInterruptTime = 0;
    unsigned long interruptTime = millis();

    if (interruptTime - lastInterruptTime > 100) {
        encoderButtonPressed = digitalRead(ROTARY_ENCODER_BUTTON_PIN);
        Serial.println("Encoder button down");
        sleepReset("Encoder button down");
    }

    lastInterruptTime = interruptTime;
}

void setupEncoder()
{
    pinMode(ROTARY_ENCODER_A_PIN, INPUT_PULLDOWN);
    pinMode(ROTARY_ENCODER_B_PIN, INPUT_PULLDOWN);
    pinMode(ROTARY_ENCODER_BUTTON_PIN, INPUT_PULLDOWN);
    
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_A_PIN), readEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_B_PIN), readEncoderISR, CHANGE);
    attachInterrupt(digitalPinToInterrupt(ROTARY_ENCODER_BUTTON_PIN), readButtonISR, RISING);
    Serial.println(F("Encoder setup"));
}