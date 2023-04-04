#pragma once
#include <Arduino.h>
#include "config.h"


volatile bool btn_up_pressed = false;
volatile bool btn_down_pressed = false;
volatile bool btn_middle_pressed = false;

unsigned long isr_debounce_time = 0;
#define DEBOUNCE_MIN_INTERVAL_MS 100

void IRAM_ATTR btn_up_isr() {
  auto t = millis();
  if (millis() - isr_debounce_time > DEBOUNCE_MIN_INTERVAL_MS) {
    btn_up_pressed = true;
    isr_debounce_time = t;
  }
}

void IRAM_ATTR btn_down_isr() {
  auto t = millis();
  if (millis() - isr_debounce_time > DEBOUNCE_MIN_INTERVAL_MS) {
    btn_down_pressed = true;
    isr_debounce_time = t;
  }
}

void IRAM_ATTR btn_middle_isr() {
  auto t = millis();
  if (millis() - isr_debounce_time > DEBOUNCE_MIN_INTERVAL_MS) {
    btn_middle_pressed = true;
    isr_debounce_time = t;
  }
}

