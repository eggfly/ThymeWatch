#pragma once

#include <Arduino.h>

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  float result;
  result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  return result;
}


float calc_real_vbat(uint16_t sensor_value) {
  float real_vbat = sensor_value * 3.3 / 4095.0 * 2.0 * 0.8681 + 0.0987;
  return real_vbat;
}
