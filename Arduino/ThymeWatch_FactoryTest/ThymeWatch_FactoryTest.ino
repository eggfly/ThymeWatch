#include "display.h"
#include <esp_sleep.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

#include "frame_00.h"
#include "frame_01.h"
#include "frame_02.h"
#include "frame_03.h"
#include "frame_04.h"
#include "frame_05.h"
#include "frame_06.h"
#include "frame_07.h"
#include "frame_08.h"
#include "frame_09.h"
#include "frame_10.h"
#include "frame_11.h"

#include "round_face_1.h"
#include "macos_face.h"
#include "macos_face_color.h"
#include "_8colors.h"

#include "rtc.h"
#include "flappy_bird.h"
#include "t_rex.h"

#include <Fonts/FreeMono24pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include "Fonts/LECO_1976_Regular1pt7b.h"
#include "Fonts/LECO_1976_Regular2pt7b.h"
#include "Fonts/LECO_1976_Regular3pt7b.h"
#include "Fonts/LECO_1976_Regular4pt7b.h"
#include "Fonts/LECO_1976_Regular5pt7b.h"
#include "Fonts/LECO_1976_Regular6pt7b.h"
#include "Fonts/LECO_1976_Regular7pt7b.h"
#include "Fonts/LECO_1976_Regular8pt7b.h"
#include "Fonts/LECO_1976_Regular9pt7b.h"
#include "Fonts/LECO_1976_Regular10pt7b.h"
#include "Fonts/LECO_1976_Regular20pt7b.h"
#include "Fonts/LECO_1976_Regular38pt7b.h"

#include "config.h"
#include "icons.h"
#include "color_defs.h"

#define ENABLE_DEEP_SLEEP 1

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */


// Set the size of the display here, e.g. 144x168!
// Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 144, 168);
// The currently-available SHARP Memory Display (144x168 pixels)
// requires > 4K of microcontroller RAM; it WILL NOT WORK on Arduino Uno
// or other <4K "classic" devices!  The original display (96x96 pixels)
// does work there, but is no longer produced.

#define BLACK 0
#define WHITE 1

// BGR order
#define THREE_BIT_BLACK   0b000
#define THREE_BIT_WHITE   0b111
#define THREE_BIT_RED     0b001
#define THREE_BIT_GREEN   0b010
#define THREE_BIT_BLUE    0b100
#define THREE_BIT_YELLOW  0b011

int minorHalfSize; // 1/2 of lesser of display width or height


RTC_DATA_ATTR char stringToKeep[20];

long boot_time;

RTC_DATA_ATTR uint32_t last_cost;

const uint8_t * nyan_frames[] = {
  nyan_128x93_frame_00,
  nyan_128x93_frame_01,
  nyan_128x93_frame_02,
  nyan_128x93_frame_03,
  nyan_128x93_frame_04,
  nyan_128x93_frame_05,
  nyan_128x93_frame_06,
  nyan_128x93_frame_07,
  nyan_128x93_frame_08,
  nyan_128x93_frame_09,
  nyan_128x93_frame_10,
  nyan_128x93_frame_11,
};

void show_colors() {
  uint8_t color = BLACK;
  for (int i = 0; i < minorHalfSize; i += 8) {
    // alternate colors
    canvas.fillRect(i, i, canvas.width() - i * 2, canvas.height() - i * 2, color & 0b111);
    canvas.refresh();
    color++;
  }
  delay(10000);
}

void show_anim() {
  canvas.clearDisplay();
  while (1) {
    for ( size_t i = 0; i < sizeof(nyan_frames) / sizeof(nyan_frames[0]); i++) {
      canvas.set3BitDrawPixelMode(true);
      canvas.drawRGBBitmap(0, 0, (uint16_t*)nyan_frames[i], 128, 93);
      canvas.set3BitDrawPixelMode(false);
      canvas.refresh();
      // delay(100);
    }
  }
}


float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  float result;
  result = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  return result;
}

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


void show_macos_time_8_colors() {
  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(true);
  canvas.drawRGBBitmap(0, 0, (uint16_t*)macos_face_color_128x128, 128, 128);
  // canvas.drawRGBBitmap(0, 0, (uint16_t*)_8colors_map, 128, 128);
  canvas.set3BitDrawPixelMode(false);

  canvas.setTextSize(1);
  canvas.setTextColor(THREE_BIT_BLACK);
  char time_str[9];  // 22:35:27\0
  char date_str[11];  // 20230323\0
  DateTime now = rtc.now();
  snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  snprintf(date_str, sizeof(date_str), "%04d%02d%02d", now.year(), now.month(), now.day());

  canvas.setCursor(10, 53);
  canvas.setFont(&LECO_1976_Regular9pt7b);
  canvas.print(time_str);
  canvas.setCursor(9, 70);
  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.print(date_str);
  canvas.setCursor(9, 82);
  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.print(daysOfTheWeek[now.dayOfTheWeek()]);

  char vbat_str[10];
  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float vbat = mapf(sensorValue, 0, 4095, 0, 3.3) * 2;
  float real_vbat = 0.8681 * vbat + 0.0987;
  canvas.setTextColor(THREE_BIT_BLACK);
  snprintf(vbat_str, sizeof(vbat_str), "%.1f%%", real_vbat);
  canvas.setCursor(100, 12);
  canvas.setFont(nullptr);
  canvas.print(vbat_str);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  //  canvas.printf("%.4f", real_vbat);
  //  canvas.println("V");
  //  canvas.print(last_cost);
  //  canvas.println("ms");
  canvas.refresh();
  last_cost = millis() - boot_time;
}


void show_timeline_watch_face() {
  DateTime now = rtc.now();
  char hour_str[3];
  snprintf(hour_str, sizeof(hour_str) , "%02d", now.hour());
  // canvas.println(buf);

  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(false);
  canvas.fillRect(0, 80, 128, 48, THREE_BIT_GREEN);
  canvas.fillTriangle(58, 80, 68, 80, 63, 85, THREE_BIT_WHITE);
  canvas.setFont(&LECO_1976_Regular38pt7b);
  canvas.setTextColor(THREE_BIT_BLACK);
  canvas.setCursor(22, 65);
  canvas.print(hour_str);
  size_t margin_and_line = 6;
  uint8_t mark_height = 7;
  for (size_t i = 0; i < SCREEN_WIDTH / margin_and_line; i++) {
    uint16_t x = i * margin_and_line;
    canvas.drawFastVLine(x, SCREEN_HEIGHT - mark_height, mark_height, THREE_BIT_WHITE);
  }
  canvas.refresh();
}

void show_round_watch_face() {
  DateTime now = rtc.now();
  char hour_str[3];
  snprintf(hour_str, sizeof(hour_str) , "%02d", now.hour());
  // canvas.println(buf);

  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(true);
  canvas.drawRGBBitmap(24, 24, (uint16_t*)round_face_80x80_map, 80, 80);
  canvas.set3BitDrawPixelMode(false);

  //
  canvas.print(hour_str);
  for (size_t i = 0; i < SCREEN_WIDTH ; i++) {
    //    uint16_t x = i * margin_and_line;
    //    canvas.drawFastVLine(x, SCREEN_HEIGHT - mark_height, mark_height, THREE_BIT_WHITE);
  }
  canvas.refresh();
}

void show_round_watch_face_pointer() {
  DateTime now = rtc.now();
  char hour_str[3];
  snprintf(hour_str, sizeof(hour_str) , "%02d", now.hour());
  // canvas.println(buf);

  canvas.clearDisplay();
  // canvas.fillScreen(THREE_BIT_BLACK);
  canvas.set3BitDrawPixelMode(false);
  float r = 60;
  uint8_t centerX = 63;
  uint8_t centerY = 63;
  canvas.drawCircle(centerX, centerY, r, THREE_BIT_BLACK);
  canvas.drawCircle(centerX, centerY, r - 1, THREE_BIT_BLACK);
  // canvas.drawLine(63, 63, 63 + , 63, THREE_BIT_WHITE);

  int angleIncrement = 30;
  uint8_t color = 0;
  for (int angle = 0; angle < 360; angle += angleIncrement) {
    int endX = centerX + round(r * cos(angle * PI / 180));
    int endY = centerY + round(r * sin(angle * PI / 180));
    // Serial.printf("%d, %d\n", endX, endY);
    canvas.drawLine(centerX, centerY, endX, endY, THREE_BIT_BLACK);
  }
  // canvas.print(hour_str);
  for (size_t i = 0; i < SCREEN_WIDTH ; i++) {
    //    uint16_t x = i * margin_and_line;
    //    canvas.drawFastVLine(x, SCREEN_HEIGHT - mark_height, mark_height, THREE_BIT_WHITE);
  }
  canvas.refresh();
  delay(500000);
}

void show_home_menu() {
  uint8_t themeColor = THREE_BIT_BLUE;
  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.fillScreen(THREE_BIT_BLACK);
  canvas.fillRect(0, 0, 128, 20, themeColor);
  canvas.setCursor(52, 18);
  canvas.setTextColor(THREE_BIT_WHITE);
  int16_t str_x, str_y;
  uint16_t str_w, str_h;
  canvas.getTextBounds("17:11", 0, 0, &str_x, &str_y, &str_w, &str_h);
  Serial.printf("bounds: %d %d %d %d\n", str_x, str_y, str_w, str_h);
  canvas.print("17:11");
  canvas.fillRect(0, 21, 128, 44, themeColor);
  canvas.drawXBitmap(127 - 20 - 4, 2, battery_charging_20x7, 20, 7, THREE_BIT_WHITE);
  canvas.drawXBitmap(8, 33, pref_20x20, 20, 20, THREE_BIT_WHITE);
  canvas.refresh();
}

#define MAX_TRIES      100
#define CIRCLE_COUNT   16
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
    // Keep trying to generate a non-overlapping circle until we reach the maximum number of tries
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

      // If the circle does not overlap, add it to the array of circles and draw it on the screen
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
    // If we could not generate a non-overlapping circle after the maximum number of tries, break out of the loop
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

  DateTime now = rtc.now();
  char time_str[9];
  if (show_seconds) {
    snprintf(time_str, sizeof(time_str) , "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  } else {
    snprintf(time_str, sizeof(time_str) , "%02d:%02d", now.hour(), now.minute());
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
    canvas.setCursor(20, 105);
    canvas.setFont(nullptr);
    // canvas.print("Press button to unlock ->");
  }
  canvas.setCursor(15, 96);
  canvas.setFont(&LECO_1976_Regular9pt7b);
  canvas.print("SAT 25 MAR");
  canvas.drawXBitmap(110, 100, unlock_xbm_10x13, 10, 13, THREE_BIT_WHITE);
  canvas.drawXBitmap(121, 102, arrow_right_7x8, 7, 8, THREE_BIT_WHITE);
  canvas.drawXBitmap(127 - 23, 5, battery_charging_23x12, 23, 12, THREE_BIT_WHITE);
  canvas.drawXBitmap(127 - 23, 25, battery_100_23x12, 23, 12, THREE_BIT_WHITE);
  canvas.refresh();
  delay(200);
}


void show_macos_time() {
  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(true);
  canvas.drawRGBBitmap(0, 0, (uint16_t*)macos_128x128, 128, 128);
  canvas.set3BitDrawPixelMode(false);

  canvas.fillRect(3, 37, 107, 33, 0b100);
  canvas.setCursor(3, 37 + 25);
  canvas.setTextSize(1);
  // canvas.setFont(&FreeMono24pt7b);
  canvas.setFont(&FreeMonoBold12pt7b);
  // canvas.setFont();
  canvas.setTextColor(0b111);
  char buf[16];
  DateTime now = rtc.now();
  snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  canvas.println(buf);
  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float vbat = mapf(sensorValue, 0, 4095, 0, 3.3) * 2;
  float real_vbat = 0.8681 * vbat + 0.0987;
  canvas.setTextColor(0b000);
  canvas.printf("%.4f", real_vbat);
  canvas.println("V");
  canvas.print(last_cost);
  canvas.println("ms");
  // canvas.drawRGBBitmap(0, 0, canvas1.getBuffer(), 32, 32);
  canvas.refresh();
  last_cost = millis() - boot_time;
}


void show_wheel_time() {
  canvas.clearDisplay();
  canvas.fillScreen(THREE_BIT_BLUE);
  canvas.fillRect(3, 37, 107, 33, THREE_BIT_GREEN);
  canvas.setCursor(3, 37 + 25);
  canvas.setTextSize(1);
  canvas.setFont(&LECO_1976_Regular38pt7b);
  canvas.setTextColor(THREE_BIT_WHITE);
  DateTime now = rtc.now();
  char hours_str[3];
  snprintf(hours_str, sizeof(hours_str), "%02d", now.hour());
  char buf[16];
  snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
  canvas.println(buf);
  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float vbat = mapf(sensorValue, 0, 4095, 0, 3.3) * 2;
  float real_vbat = 0.8681 * vbat + 0.0987;
  canvas.setTextColor(0b000);
  canvas.printf("%.4f", real_vbat);
  canvas.println("V");
  canvas.print(last_cost);
  canvas.println("ms");
  canvas.refresh();
  last_cost = millis() - boot_time;
}

void show_time() {
  DateTime now = rtc.now();

  canvas.setRotation(0);
  canvas.clearDisplay();

  canvas.setTextSize(1);
  canvas.setTextColor(BLACK);
  canvas.setCursor(0, 0);
  canvas.println("Hello, world!");
  canvas.setTextColor(WHITE, BLACK); // inverted text
  canvas.println(3.141592);
  canvas.setTextSize(2);
  canvas.setTextColor(BLACK);
  canvas.print("0x"); canvas.println(0xDEADBEEF, HEX);
  canvas.print("cost: "); canvas.println(last_cost, HEX);

  canvas.print(now.year(), DEC);
  canvas.print('/');
  canvas.print(now.month(), DEC);
  canvas.print('/');
  canvas.println(now.day(), DEC);

  canvas.print(now.hour(), DEC);
  canvas.print(':');
  canvas.print(now.minute(), DEC);
  canvas.print(':');
  canvas.print(now.second(), DEC);
  canvas.println();

  // sprintf(stringToKeep, "string");
  canvas.refresh();
}

bool btn_up_pressed_on_boot = false;
bool btn_down_pressed_on_boot = false;

void setup(void) {
  boot_time = millis();
  btn_up_pressed_on_boot = !digitalRead(WAKEUP_PIN_UP);
  btn_down_pressed_on_boot = !digitalRead(WAKEUP_PIN_DOWN);
  //  if (!ENABLE_DEEP_SLEEP) {
  Serial.begin(115200);
  //  }
  pinMode(SHARP_DISP, OUTPUT);
  digitalWrite(SHARP_DISP, HIGH);
  SPI.begin(SHARP_SCK, 11, SHARP_MOSI, SHARP_SS);

  // start & clear the display
  canvas.begin();
  canvas.clearDisplay();
  // Several shapes are drawn centered on the screen.  Calculate 1/2 of
  // lesser of display width or height, this is used repeatedly later.
  minorHalfSize = min(canvas.width(), canvas.height()) / 2;
  init_rtc();
  if (!ENABLE_DEEP_SLEEP) {
    show_datetime();
  }
  // show_anim();
  // show_time();
  // canvas.setRotation(0);
  // t_rex_setup();
}



void loop(void) {
  // flappy_bird_loop();
  //  while (1) {
  //    t_rex_loop();
  //    delay(1);
  //  }
  // flappy_bird_loop();
  show_macos_time();
  // show_macos_time_8_colors();
  // show_timeline_watch_face();
  // show_round_watch_face();
  // show_round_watch_face_pointer();
  // show_wheel_time();
  // show_random_ring_face(true);
  //show_home_menu();
  if (ENABLE_DEEP_SLEEP) {
    for (int i = 0; i < 100; i++) {
      delay(500);
      //show_home_menu();
      show_macos_time();
    }
    show_random_ring_face(false);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(
                      BIT(WAKEUP_PIN_UP) | BIT(WAKEUP_PIN_DOWN), DEFAULT_WAKEUP_LEVEL));
    Serial.println("--- NOW GOING TO SLEEP! ---");
    // esp_deep_sleep_enable_gpio_wakeup();
    esp_deep_sleep_start();
  } else {
    show_random_ring_face(true);
    Serial.println(last_cost);
    delay(900);
  }
}
