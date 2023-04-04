#include <Wire.h>
#include "pcf8563.h"

const uint8_t INT_PIN = 0;
const uint8_t IO1_PIN = 1;

const uint8_t BTN_UP = 4;
const uint8_t DISP = 10;

// flag to update serial; set in interrupt callback
volatile uint8_t tick_tock = 1;

// INT0 interrupt callback; update tick_tock flag
void set_tick_tock(void) {
  tick_tock = 1;
}

PCF8563_Class rtc;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 20);
  pinMode(INT_PIN, INPUT_PULLUP);
  pinMode(IO1_PIN, INPUT_PULLDOWN);
  // pinMode(BTN_UP, INPUT_PULLDOWN);
  // digitalWrite(INT_PIN, HIGH);
  // digitalWrite(IO1_PIN, HIGH);

  attachInterrupt(digitalPinToInterrupt(INT_PIN), set_tick_tock, CHANGE);
  attachInterrupt(INT_PIN, set_tick_tock, CHANGE);

  rtc.begin();
  // year, month, day, hour, minute, second
  rtc.setDateTime(2019, 4, 1, 12, 2, 55);
  rtc.resetAlarm();
  rtc.disableTimer();
  rtc.setAlarmByMinutes(3);
  rtc.enableAlarm();
  // rtc.enableTimer();
  Serial.println("setup() ok");
}


void loop() {
  // Serial.println(rtc.formatDateTime(PCF_TIMEFORMAT_YYYY_MM_DD_H_M_S));
  delay(1000);
  //  Serial.printf("%d %d %d %d ", rtc.alarmActive(), rtc.isTimerEnable(),
  //                rtc.isTimerActive(), rtc.status2());
  Serial.printf("%d %d %d\r\n",
                digitalRead(INT_PIN), digitalRead(IO1_PIN), digitalRead(DISP));
  for (size_t i = 0; i < 10; i++) {
    // Serial.printf("%d=%d\r\n", i, digitalRead(i));
  }
  Serial.println("delay 1000");
  if (tick_tock) {
    Serial.println("tick_tock!!");
    tick_tock = 0;
  }
}
