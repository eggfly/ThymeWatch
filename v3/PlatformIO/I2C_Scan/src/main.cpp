
#include <Arduino.h>

#include <Wire.h>

#define TP_SDA 41
#define TP_SCL 42

#define ULP_SDA 3
#define ULP_SCL 2

void setup() {
  pinMode(47, OUTPUT);
  digitalWrite(47, HIGH);
  Serial.begin(115200);
  delay(2000);
  Wire.begin(ULP_SDA, ULP_SCL);   // sda, scl reuse uart pins
  Serial.println("I2C Scanner");
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
