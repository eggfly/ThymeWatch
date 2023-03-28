#pragma once

#include "Adafruit_GFX.h"

typedef enum {
  HORIZONTAL_LEFT = 0,
  HORIZONTAL_CENTER,
  HORIZONTAL_RIGHT,
} my_gfx_horizontal_align_t;

typedef enum {
  VERTICAL_TOP = 0,
  VERTICAL_MIDDLE,
  VERTICAL_BOTTOM,
} my_gfx_vertical_align_t;

void drawTextWithAlignment(Adafruit_GFX *gfx, const char *text, int16_t x,
                           int16_t y, my_gfx_horizontal_align_t horizontal,
                           my_gfx_vertical_align_t vertical) {
  // gfx->setCursor(0, 0);
  int16_t out_x, out_y;
  uint16_t out_w, out_h;
  gfx->getTextBounds(text, 0, 0, &out_x, &out_y, &out_w, &out_h);
  Serial.printf("Bounds: %d %d %d %d\n", out_x, out_y, out_w, out_h);
  if (horizontal == HORIZONTAL_CENTER && vertical == VERTICAL_MIDDLE) {
    int16_t real_x = x - out_w / 2;
    int16_t real_y = y + out_h / 2;
    gfx->setCursor(real_x, real_y);
    gfx->print(text);
    // gfx->drawRect(out_x, out_y, out_w, out_h, 0b011);
  }
}