#include <Arduino.h>
#include "Wire.h"
#include <driver/rtc_io.h>

#include <Adafruit_INA219.h>

// LED ANODE -> IO5

Adafruit_INA219 ina219;

#define ENABLE_DEEP_SLEEP 1
#define DEFAULT_WAKEUP_LEVEL ESP_GPIO_WAKEUP_GPIO_LOW
#define WAKEUP_PIN_UP 4
#define WAKEUP_PIN_DOWN 9

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 10       /* Time ESP32 will go to sleep (in seconds) */


void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Hello, World!");
  delay(7000);
  if (ENABLE_DEEP_SLEEP)
  {
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(
        BIT(WAKEUP_PIN_UP) | BIT(WAKEUP_PIN_UP), DEFAULT_WAKEUP_LEVEL));
    Serial.println("--- NOW GOING TO SLEEP! ---");
    // esp_deep_sleep_enable_gpio_wakeup();
    esp_deep_sleep_start();
  }
}

void loop()
{
  Serial.println("Hello, World!");
  delay(1000);
  // put your main code here, to run repeatedly:
}
