#include <config.h>

extern bool ptt_pressed;

SX1262 radio = new Module(LORA_RADIO_PIN_SS, LORA_RADIO_PIN_A, LORA_RADIO_PIN_RST, LORA_RADIO_PIN_B);

TaskHandle_t audio_task_handle;       // audio playback/record task
TaskHandle_t lora_task_handle;        // lora rx/tx task

extern Timer<1> light_sleep_timer;
extern bool light_sleep(void *param);
extern void light_sleep_reset();
extern void onLoraDataAvailableIsr();
extern void audio_task(void *param);
extern void lora_task(void *param);
extern void setupAudio();

void setup() {
  LOG_SET_LEVEL(DebugLogLevel::LVL_INFO);

  Serial.begin(SERIAL_BAUD_RATE);
  while (!Serial);

  delay(1000); // wait for platformio's serialmon window change

  LOG_INFO("Board setup started");

  // setup ptt button
  pinMode(PTTBTN_PIN, INPUT);

  int state = radio.begin(LORA_RADIO_FREQ, LORA_RADIO_BW, LORA_RADIO_SF, LORA_RADIO_CR, LORA_RADIO_SYNC, LORA_RADIO_PWR, LORA_RADIO_PL, 0, false);
  if (state != RADIOLIB_ERR_NONE) {
    LOG_ERROR("Lora radio start failed:", state);
    for(;;) { delay(100); };
  }
  
  LOG_INFO("Lora radio initialized");
  radio.setCRC(LORA_RADIO_CRC);
  radio.setRfSwitchPins(LORA_RADIO_PIN_RXEN, LORA_RADIO_PIN_TXEN);
  radio.clearDio1Action();
  radio.setDio1Action(onLoraDataAvailableIsr);
#ifdef LORA_RADIO_EXPL 
  LOG_INFO("Using explicit header");
  radio.explicitHeader();
#else
  LOG_INFO("Using implicit header");
  radio.implicitHeader();
#endif

  setupAudio();

  xTaskCreate(&audio_task,  "audio_task", 32000,  NULL, 5, &audio_task_handle);
  xTaskCreate(&lora_task,   "lora_task",  8000,   NULL, 5, &lora_task_handle);

  state = radio.startReceive();
  if (state != RADIOLIB_ERR_NONE) {
    LOG_ERROR("Receive start error:", state);
  }
  LOG_INFO("Board setup completed");

#ifdef ENABLE_LIGHT_SLEEP
  LOG_INFO("Light sleep is enabled");
#endif
  light_sleep_reset();
}

void loop() {
  bool ptt_state = (analogRead(PTTBTN_PIN) == 4095);

  if (ptt_state && !ptt_pressed) {
    LOG_INFO("PTT pushed, start TX");
    ptt_pressed = true;

    // notify to start recording
    xTaskNotify(audio_task_handle, AUDIO_TASK_RECORD_BIT, eSetBits);
  } else if (!ptt_state && ptt_pressed) {
    LOG_INFO("PTT released");
    ptt_pressed = false;
  }
  light_sleep_timer.tick();
  delay(50);
}