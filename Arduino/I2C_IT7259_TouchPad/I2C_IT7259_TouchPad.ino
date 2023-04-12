
// ESP32 I2C Scanner
// Based on code of Nick Gammon  http://www.gammon.com.au/forum/?id=10896
// ESP32 DevKit - Arduino IDE 1.8.5
// Device tested PCF8574 - Use pullup resistors 3K3 ohms !
// PCF8574 Default Freq 100 KHz

#include <Wire.h>
#include "IT7259Driver.h"

#define I2C_ADDR 0x46

void setup() {
  Serial.begin(115200);
  pinMode(5, OUTPUT);
  Wire.begin(2, 3);   // sda= /scl=
}


void i2c_tx(uint8_t* buf, size_t len) {
  // begin tx
  Wire.beginTransmission(I2C_ADDR);
  // data, length
  Wire.write(buf, len);
  //0: success.
  //1: data too long to fit in transmit buffer.
  //2: received NACK on transmit of address.
  //3: received NACK on transmit of data.
  //4: other error.
  //5: timeout
  auto ret = Wire.endTransmission(true);
  Serial.printf("tx_len=%d, tx_ret=%d,\n", len, ret);
  delay(50);
}


void i2c_rx(uint8_t* buf, size_t len) {
  // begin rx
  size_t n = Wire.requestFrom((uint8_t)I2C_ADDR, (uint8_t)len);
  // x = Wire.read();
  for (size_t i = 0; i < len; i++) {
    buf[i] = Wire.read();
    Serial.printf("0x%02x('%c') ", buf[i], buf[i]);
    delay(10);
  }
  while (Wire.available()) {
    Wire.read();
  }
  Serial.println();
  Serial.printf("rx_len=%d request size: %d\n", len, n);
}


void it7259_device_name() {
  Serial.println();
  Serial.println("device name...");
  uint8_t tx_buf[] = {0x20, 0x00,};
  uint8_t rx_buf[32];
  // send 0x20 command
  i2c_tx(tx_buf, 2);
  // send 0xa0 read from response buffer
  tx_buf[0] = 0xA0;
  i2c_tx(tx_buf, 1);
  // read response
  i2c_rx(rx_buf, 10);
}


void it7259_firmware_version () {
  Serial.println();
  Serial.println("firmware version...");
  uint8_t tx_buf[] = {0x20, 0x01, 0x00,};
  uint8_t rx_buf[32];
  // send 0x20 command
  i2c_tx(tx_buf, 3);
  // send 0xa0 read from response buffer
  tx_buf[0] = 0xA0;
  i2c_tx(tx_buf, 1);
  // read response
  i2c_rx(rx_buf, 9);
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
