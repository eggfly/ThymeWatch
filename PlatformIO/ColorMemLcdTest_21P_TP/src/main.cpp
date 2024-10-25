#include <Arduino.h>
#include "Wire.h"
#include <driver/rtc_io.h>

#define IO4_PIN 4 // 定义 IO4 的引脚号

// put function definitions here:
int myFunction(int x, int y)
{
  return x + y;
}

void setup2()
{
  Serial.begin(115200);
  pinMode(IO4_PIN, INPUT);
  if (digitalRead(IO4_PIN) == LOW)
  {
    delay(3000);
    Serial.println("----------------- 哈哈，想不用飞线了，一定要用我这段代码 -----------------");
    Serial.println("IO4 is LOW, will restart and force to download mode...");
    REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT_V);
    esp_restart();
  }
  // while(1);
  delay(1000);
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop2()
{
  Serial.println("Hello, World!");
  delay(1000);
  // put your main code here, to run repeatedly:
}
