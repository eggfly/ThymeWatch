#include <Arduino.h>
#include "Wire.h"
#include <driver/rtc_io.h>

#include <Adafruit_INA219.h>

// LED ANODE -> IO5

#define WAKEUP_TP_INT 0
#define WAKEUP_IMU_INT 1
#define WAKEUP_PIN_UP 4

Adafruit_INA219 ina219;

#define ENABLE_DEEP_SLEEP 1
#define DEFAULT_WAKEUP_LEVEL ESP_GPIO_WAKEUP_GPIO_LOW


#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 30       /* Time ESP32 will go to sleep (in seconds) */

#define TP_RESET 10

void setup()
{
  Serial.begin(115200);
  delay(3000);
  Serial.println("Hello, World!");
  delay(7000);

  pinMode(TP_RESET, OUTPUT);
  digitalWrite(TP_RESET, HIGH);

  if (ENABLE_DEEP_SLEEP)
  {
    gpio_set_direction((gpio_num_t)WAKEUP_TP_INT, GPIO_MODE_INPUT);
    gpio_set_direction((gpio_num_t)WAKEUP_IMU_INT, GPIO_MODE_INPUT);

    gpio_sleep_set_direction((gpio_num_t)WAKEUP_TP_INT, GPIO_MODE_INPUT);
    gpio_sleep_set_direction((gpio_num_t)WAKEUP_IMU_INT, GPIO_MODE_INPUT);
    gpio_sleep_set_direction((gpio_num_t)WAKEUP_PIN_UP, GPIO_MODE_INPUT);

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
