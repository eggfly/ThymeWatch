
#include <Arduino.h>
#include <opt3001.h>
#include <Wire.h>

opt3001 sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 20);   // sda, scl reuse uart pins
  delay(2000);
  sensor.setup(Wire, 0x44);
  auto success = sensor.detect() == 0;
  Serial.printf("OPT3001 detected: %s\n", success ? "true" : "false");
  sensor.config_set(OPT3001_CONVERSION_TIME_100MS);
}


void loop() {
  float lux;
  sensor.conversion_continuous_enable();
  delay(200);
  auto ok = sensor.lux_read(&lux) == 0;
  Serial.printf("Lux: %.2f, success=%s\n", lux, ok ? "true" : "false");
}
