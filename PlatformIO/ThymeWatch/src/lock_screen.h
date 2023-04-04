#pragma once

#include "display.h"
#include "colors.h"
#include "icons.h"
#include "fonts.h"
#include "rtc.h"
#include "config.h"


#define MAX_TRIES 100
#define CIRCLE_COUNT 16

struct Circle {
  int x;
  int y;
  int radius;
};

// Create an array to hold the circles
Circle circles[CIRCLE_COUNT];


void show_random_ring_face(bool show_seconds) {
  canvas.fillScreen(THREE_BIT_BLACK);
  canvas.set3BitDrawPixelMode(false);

  uint8_t centerX = 63;
  uint8_t centerY = 63;
  uint8_t min_radius = 4;
  uint8_t max_radius = 24;
  // canvas.drawCircle(centerX, centerY, r, THREE_BIT_BLACK);
  // canvas.drawCircle(centerX, centerY, r - 1, THREE_BIT_BLACK);
  uint8_t color_base = random(0, 6);
  for (int i = 0; i < CIRCLE_COUNT; i++) {
    // Keep trying to generate a non-overlapping circle until we reach the
    // maximum number of tries
    int tries = 0;
    bool overlap = true;
    while (overlap && tries < MAX_TRIES) {
      // Generate a random circle with a radius between 5 and 20
      int x = random(-min_radius, SCREEN_WIDTH + min_radius);
      int y = random(-min_radius, SCREEN_HEIGHT + min_radius);
      int radius = random(min_radius, max_radius + 1);

      // Check if this circle overlaps with any of the previous circles
      overlap = false;
      for (int j = 0; j < i; j++) {
        int dx = circles[j].x - x;
        int dy = circles[j].y - y;
        int distance = sqrt(dx * dx + dy * dy);
        if (distance < (circles[j].radius + radius)) {
          overlap = true;
          break;
        }
      }

      // If the circle does not overlap, add it to the array of circles and draw
      // it on the screen
      if (!overlap) {
        circles[i] = {x, y, radius};
        // tft.drawCircle(x, y, radius, ST77XX_WHITE);
        // exclude white and black
        canvas.drawCircle(x, y, radius, color_base + 1);
        canvas.drawCircle(x, y, radius - 1, color_base + 1);
        canvas.drawCircle(x, y, radius - 2, color_base + 1);
        color_base++;
        color_base %= 6;
      }
      tries++;
    }
    // If we could not generate a non-overlapping circle after the maximum
    // number of tries, break out of the loop
    if (tries == MAX_TRIES) {
      break;
    }
  }
  //  for (int i = 0; i < 10; i++) {
  //    int x = random(0, SCREEN_WIDTH);
  //    int y = random(0, SCREEN_HEIGHT);
  //    int radius = random(4, 24);
  //    canvas.drawCircle(x, y, radius, THREE_BIT_YELLOW);
  //    canvas.drawCircle(x, y, radius - 1, THREE_BIT_YELLOW);
  //    canvas.drawCircle(x, y, radius - 2, THREE_BIT_YELLOW);
  //  }
#if defined(ENABLE_RTC)
  DateTime now = rtc.now();
  char time_str[9];
  if (show_seconds) {
    snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", now.hour(),
             now.minute(), now.second());
  } else {
    snprintf(time_str, sizeof(time_str), "%02d:%02d", now.hour(), now.minute());
  }

  canvas.setTextColor(THREE_BIT_WHITE);
  if (show_seconds) {
    canvas.setCursor(11, 65);
    canvas.setFont(&LECO_1976_Regular10pt7b);
    canvas.print(time_str);
  } else {
    canvas.setCursor(11, 65);
    canvas.setFont(&LECO_1976_Regular20pt7b);
    canvas.print(time_str);
    // canvas.setFont(&LECO_1976_Regular4pt7b);
    // canvas.setCursor(20, 105);
    // canvas.setFont(nullptr);
    // canvas.print("Press button to unlock ->");
  }
#endif
  canvas.setCursor(15, 96);
  canvas.setFont(&LECO_1976_Regular9pt7b);
  canvas.print("SAT 25 MAR");
  canvas.drawXBitmap(110, 100, unlock_xbm_10x13, 10, 13, THREE_BIT_WHITE);
  canvas.drawXBitmap(121, 102, arrow_right_7x8, 7, 8, THREE_BIT_WHITE);
  canvas.drawXBitmap(127 - 23, 5, battery_charging_23x12, 23, 12,
                     THREE_BIT_WHITE);
  canvas.drawXBitmap(127 - 23, 25, battery_100_23x12, 23, 12, THREE_BIT_WHITE);
  canvas.refresh();
}
