
#include <Arduino.h>

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 20);   // sda, scl reuse uart pins
}

void Scanner() {
  Serial.println();
  Serial.println("I2C scanner. Scanning ...");
  byte count = 0;

  for (byte i = 8; i < 120; i++) {
    Wire.beginTransmission(i);          // Begin I2C transmission Address (i)
    if (Wire.endTransmission() == 0)  // Receive 0 = success (ACK response)
    {
      Serial.print("Found address: ");
      Serial.print(i, DEC);
      Serial.print(" (0x");
      Serial.print(i, HEX);     // PCF8574 7 bit address
      Serial.println(")");
      count++;
    }
  }
  Serial.print("Found ");
  Serial.print(count, DEC);        // numbers of devices
  Serial.println(" device(s).");
}

void loop() {
  Scanner();
  delay(1000);
}
