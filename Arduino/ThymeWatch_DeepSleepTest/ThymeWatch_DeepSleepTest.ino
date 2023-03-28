
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

RTC_DATA_ATTR char stringToKeep[20];

long boot_time;

RTC_DATA_ATTR uint32_t last_cost;

bool boot_pressed_at_startup;

void setup(void) {
  pinMode(9, INPUT);
  boot_pressed_at_startup = !digitalRead(9);
  boot_time = millis();
  Serial.begin(115200);
  delay(500);
  Serial.println("hello");
}

void loop(void) {
  Serial.println(last_cost);
  delay(500);
  if (!boot_pressed_at_startup) {
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }
}
