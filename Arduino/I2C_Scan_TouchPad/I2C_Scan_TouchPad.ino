
// ESP32 I2C Scanner
// Based on code of Nick Gammon  http://www.gammon.com.au/forum/?id=10896
// ESP32 DevKit - Arduino IDE 1.8.5
// Device tested PCF8574 - Use pullup resistors 3K3 ohms !
// PCF8574 Default Freq 100 KHz

#include <Wire.h>

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  Wire.begin(2, 3);   // sda= GPIO_10 /scl= GPIO_3
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
  digitalWrite(5, HIGH);
  Scanner();
  delay(1000);

  // digitalWrite(5, LOW);
  Scanner();
  delay(1000);
}
