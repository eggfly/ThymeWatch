/*
    This sketch demonstrates how to scan WiFi networks.
    The API is almost the same as with the WiFi Shield library,
    the most obvious difference being the different file you need to include:
*/
#include "display.h"

#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>

#include "WiFi.h"

void setup()
{
  Serial.begin(115200);

  pinMode(SHARP_DISP, OUTPUT);
  digitalWrite(SHARP_DISP, HIGH);
  SPI.begin(SHARP_SCK, 11, SHARP_MOSI, SHARP_SS);

  // start & clear the display
  canvas.begin();


  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  float result;
  result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  return result;
}

void loop() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again
  delay(3000);

  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float vbat = mapf(sensorValue, 0, 4095, 0, 3.3) * 2;
  Serial.printf("%.4fV\n", vbat);

  canvas.clearDisplay();
  canvas.setTextSize(1);
  canvas.setFont(&FreeMonoBold12pt7b);
  canvas.setCursor(0, 20);
  canvas.setTextColor(0b000);
  canvas.printf("%.4fV\n", vbat);
  canvas.println(millis());
  canvas.refresh();
}
