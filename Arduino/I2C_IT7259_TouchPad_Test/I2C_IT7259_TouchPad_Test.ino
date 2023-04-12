
// ESP32 I2C Scanner
// Based on code of Nick Gammon  http://www.gammon.com.au/forum/?id=10896
// ESP32 DevKit - Arduino IDE 1.8.5
// Device tested PCF8574 - Use pullup resistors 3K3 ohms !
// PCF8574 Default Freq 100 KHz

#include <Wire.h>
#include "IT7259Driver.h"


void setup() {
  Serial.begin(115200);
  // tp reset
  pinMode(5, OUTPUT);
  Wire.begin(2, 3);   // sda= /scl=
}



void loop() {
  //  digitalWrite(5, LOW);
  //  delay(555);
  digitalWrite(5, HIGH);
  struct TouchPointData pointData;
  readTouchEvent(&pointData);
  if (!pointData.isNull) {
    Serial.printf("x=%d,y=%d\n", pointData.xPos, pointData.yPos);
  }
  delay(10);
  //  it7259_device_name();
  //  delay(1000);
  //  it7259_firmware_version();
  //  delay(1000);
}
