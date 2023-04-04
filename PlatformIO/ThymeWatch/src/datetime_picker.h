#pragma once

#include "colors.h"
#include "display.h"
#include "fonts.h"
#include "gfx_utils.h"

void draw_selector_item(int16_t start_x, int16_t start_y, int16_t w, int16_t h,
                        int16_t radius) {
  canvas.setFont(&RasterGothic18CondBold18pt7b);
  canvas.setTextColor(THREE_BIT_BLACK);
  canvas.drawRoundRect(start_x, start_y, w, h, radius, THREE_BIT_BLACK);
  canvas.drawRoundRect(start_x + 1, start_y + 1, w - 2, h - 2, radius,
                       THREE_BIT_BLACK);
  canvas.fillTriangle(start_x + w / 2 - 7, start_y + h + 2, start_x + w / 2 + 7,
                      start_y + h + 2, start_x + w / 2, start_y + h + 7,
                      THREE_BIT_BLACK);
  canvas.fillTriangle(start_x + w / 2 - 7, start_y - 2, start_x + w / 2 + 7,
                      start_y - 2, start_x + w / 2, start_y - 7,
                      THREE_BIT_BLACK);
  drawTextWithAlignment(&canvas, "08", start_x + w / 2, start_y + h / 2,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);
}

void show_datetime_picker() {
  canvas.fillScreen(THREE_BIT_WHITE);
  int16_t start_x = 5;
  int16_t start_y = 10;
  int16_t selector_w = 37;
  int16_t selector_h = 64;
  int16_t radius = 10;
  int16_t padding = 3;

  // Month
  canvas.drawRoundRect(start_x, start_y, selector_w, selector_h, radius,
                       THREE_BIT_BLACK);
  canvas.drawRoundRect(start_x + 1, start_y + 1, selector_w - 2, selector_h - 2,
                       radius, THREE_BIT_BLACK);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.setTextColor(THREE_BIT_BLUE);
  drawTextWithAlignment(&canvas, "SEP", start_x + selector_w / 2,
                        start_y + selector_h / 2 - selector_h / 3,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  canvas.setFont(&RasterGothic18CondBold9pt7b);
  canvas.setTextColor(THREE_BIT_BLACK);
  drawTextWithAlignment(&canvas, "OCT", start_x + selector_w / 2,
                        start_y + selector_h / 2, HORIZONTAL_CENTER,
                        VERTICAL_MIDDLE);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.setTextColor(THREE_BIT_BLUE);
  drawTextWithAlignment(&canvas, "NOV", start_x + selector_w / 2,
                        start_y + selector_h / 2 + selector_h / 3,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  start_x += selector_w + padding;
  // Date
  canvas.drawRoundRect(start_x, start_y, selector_w, selector_h, radius,
                       THREE_BIT_BLACK);
  canvas.drawRoundRect(start_x + 1, start_y + 1, selector_w - 2, selector_h - 2,
                       radius, THREE_BIT_BLACK);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.setTextColor(THREE_BIT_BLUE);
  drawTextWithAlignment(&canvas, "21", start_x + selector_w / 2,
                        start_y + selector_h / 2 - selector_h / 3,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  canvas.setFont(&RasterGothic18CondBold9pt7b);
  canvas.setTextColor(THREE_BIT_BLACK);
  drawTextWithAlignment(&canvas, "22", start_x + selector_w / 2,
                        start_y + selector_h / 2, HORIZONTAL_CENTER,
                        VERTICAL_MIDDLE);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.setTextColor(THREE_BIT_BLUE);
  drawTextWithAlignment(&canvas, "23", start_x + selector_w / 2,
                        start_y + selector_h / 2 + selector_h / 3,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  start_x += selector_w + padding;
  // Year
  canvas.drawRoundRect(start_x, start_y, selector_w, selector_h, radius,
                       THREE_BIT_BLACK);
  canvas.drawRoundRect(start_x + 1, start_y + 1, selector_w - 2, selector_h - 2,
                       radius, THREE_BIT_BLACK);
  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.setTextColor(THREE_BIT_BLUE);
  drawTextWithAlignment(&canvas, "2021", start_x + selector_w / 2,
                        start_y + selector_h / 2 - selector_h / 3,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  canvas.setFont(&RasterGothic18CondBold9pt7b);
  canvas.setTextColor(THREE_BIT_BLACK);
  drawTextWithAlignment(&canvas, "2022", start_x + selector_w / 2,
                        start_y + selector_h / 2, HORIZONTAL_CENTER,
                        VERTICAL_MIDDLE);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.setTextColor(THREE_BIT_BLUE);
  drawTextWithAlignment(&canvas, "2023", start_x + selector_w / 2,
                        start_y + selector_h / 2 + selector_h / 3,
                        HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  // Hour
  start_x = 16;
  start_y = 82;
  selector_w = 44;
  selector_h = 33;

  draw_selector_item(start_x, start_y, selector_w, selector_h, radius);
  // minute
  start_x += selector_w + 6;
  draw_selector_item(start_x, start_y, selector_w, selector_h, radius);

  canvas.refresh();
}
