#include "colors.h"
#include "display.h"
#include "lock_screen.h"
#include <esp_sleep.h>

#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

#include "res/nyan_cat_frames.h"

#include "_8colors.h"
#include "macos_face.h"
#include "macos_face_color.h"
#include "round_face_1.h"

#include "games/flappy_bird.h"
#include "rtc.h"
#include "t_rex.h"

#include "buttons.h"
#include "color_defs.h"
#include "config.h"
#include "datetime_picker.h"
#include "gfx_utils.h"
#include "icons.h"
#include "utils.h"

#define ENABLE_DEEP_SLEEP 1

/* Conversion factor for micro seconds to seconds */
#define uS_TO_S_FACTOR 1000000
/* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_SLEEP 60 * 60 * 12

// Set the size of the display here, e.g. 144x168!
// Adafruit_SharpMem display(SHARP_SCK, SHARP_MOSI, SHARP_SS, 144, 168);
// The currently-available SHARP Memory Display (144x168 pixels)
// requires > 4K of microcontroller RAM; it WILL NOT WORK on Arduino Uno
// or other <4K "classic" devices!  The original display (96x96 pixels)
// does work there, but is no longer produced.

int minorHalfSize; // 1/2 of lesser of display width or height

RTC_DATA_ATTR char stringToKeep[20];

long boot_time;

RTC_DATA_ATTR uint32_t last_cost;

const uint8_t *nyan_frames[] = {
    nyan_128x93_frame_00, nyan_128x93_frame_01, nyan_128x93_frame_02,
    nyan_128x93_frame_03, nyan_128x93_frame_04, nyan_128x93_frame_05,
    nyan_128x93_frame_06, nyan_128x93_frame_07, nyan_128x93_frame_08,
    nyan_128x93_frame_09, nyan_128x93_frame_10, nyan_128x93_frame_11,
};

void show_colors() {
  uint8_t color = BLACK;
  for (int i = 0; i < minorHalfSize; i += 8) {
    // alternate colors
    canvas.fillRect(i, i, canvas.width() - i * 2, canvas.height() - i * 2,
                    color & 0b111);
    canvas.refresh();
    color++;
  }
  delay(10000);
}

void show_anim() {
  canvas.clearDisplay();
  while (1) {
    for (size_t i = 0; i < sizeof(nyan_frames) / sizeof(nyan_frames[0]); i++) {
      canvas.set3BitDrawPixelMode(true);
      canvas.drawRGBBitmap(0, 0, (uint16_t *)nyan_frames[i], 128, 93);
      canvas.set3BitDrawPixelMode(false);
      canvas.refresh();
      // delay(100);
    }
  }
}

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday",
// "Thursday", "Friday", "Saturday"};

void show_macos_time_8_colors() {
  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(true);
  canvas.drawRGBBitmap(0, 0, (uint16_t *)macos_face_color_128x128, 128, 128);
  // canvas.drawRGBBitmap(0, 0, (uint16_t*)_8colors_map, 128, 128);
  canvas.set3BitDrawPixelMode(false);

  canvas.setTextSize(1);
  canvas.setTextColor(THREE_BIT_BLACK);
  char time_str[9];  // 22:35:27\0
  char date_str[11]; // 20230323\0
  DateTime now = rtc.now();
  snprintf(time_str, sizeof(time_str), "%02d:%02d:%02d", now.hour(),
           now.minute(), now.second());
  snprintf(date_str, sizeof(date_str), "%04d%02d%02d", now.year(), now.month(),
           now.day());

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
  float real_vbat = calc_real_vbat(sensorValue);
  canvas.setTextColor(THREE_BIT_BLACK);
  snprintf(vbat_str, sizeof(vbat_str), "%.1f%%", real_vbat);
  canvas.setCursor(100, 12);
  canvas.setFont(nullptr);
  canvas.print(vbat_str);

  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.printf("%.4f", real_vbat);
  canvas.println("V");
  canvas.print(last_cost);
  canvas.println("ms");
  canvas.refresh();
  last_cost = millis() - boot_time;
}

void show_timeline_watch_face() {
  DateTime now = rtc.now();
  char hour_str[3];
  snprintf(hour_str, sizeof(hour_str), "%02d", now.hour());
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
    canvas.drawFastVLine(x, SCREEN_HEIGHT - mark_height, mark_height,
                         THREE_BIT_WHITE);
  }
  canvas.refresh();
}

void show_round_watch_face() {
  DateTime now = rtc.now();
  char hour_str[3];
  snprintf(hour_str, sizeof(hour_str), "%02d", now.hour());
  // canvas.println(buf);

  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(true);
  canvas.drawRGBBitmap(24, 24, (uint16_t *)round_face_80x80_map, 80, 80);
  canvas.set3BitDrawPixelMode(false);

  //
  canvas.print(hour_str);
  for (size_t i = 0; i < SCREEN_WIDTH; i++) {
    //    uint16_t x = i * margin_and_line;
    //    canvas.drawFastVLine(x, SCREEN_HEIGHT - mark_height, mark_height,
    //    THREE_BIT_WHITE);
  }
  canvas.refresh();
}

void show_round_watch_face_pointer() {
  DateTime now = rtc.now();
  char hour_str[3];
  snprintf(hour_str, sizeof(hour_str), "%02d", now.hour());
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
  for (size_t i = 0; i < SCREEN_WIDTH; i++) {
    //    uint16_t x = i * margin_and_line;
    //    canvas.drawFastVLine(x, SCREEN_HEIGHT - mark_height, mark_height,
    //    THREE_BIT_WHITE);
  }
  canvas.refresh();
  delay(500000);
}

int8_t gSelectedItem = 0;

typedef struct {
  const char *title;
  const char *subtitle;
  const uint8_t *xbm_icon;
  uint16_t icon_width;
  uint16_t icon_height;
} MenuItem;

MenuItem home_menu_items[] = {
    {
        .title = "Settings",
        .subtitle = nullptr,
        .xbm_icon = pref_20x20,
        .icon_width = 20,
        .icon_height = 20,
    },
    {
        .title = "Notifications",
        .subtitle = nullptr,
        .xbm_icon = notifications_20x20,
        .icon_width = 20,
        .icon_height = 20,
    },
    {
        .title = "Time",
        .subtitle = nullptr,
        .xbm_icon = alarm_20x20,
        .icon_width = 20,
        .icon_height = 20,
    },
    {
        .title = "Alarms",
        .subtitle = nullptr,
        .xbm_icon = alarm_20x20,
        .icon_width = 20,
        .icon_height = 20,
    },
};

const size_t home_menu_items_count =
    sizeof(home_menu_items) / sizeof(home_menu_items[0]);

void show_home_menu() {
  uint8_t themeColor = THREE_BIT_BLUE;
  canvas.setFont(&LECO_1976_Regular5pt7b);
  canvas.fillScreen(THREE_BIT_WHITE);
  canvas.fillRect(0, 0, 128, 20, themeColor);
  canvas.setCursor(52, 18);
  canvas.setTextColor(THREE_BIT_WHITE);
  // int16_t str_x, str_y;
  // uint16_t str_w, str_h;
  // canvas.getTextBounds("17:11", 0, 0, &str_x, &str_y, &str_w, &str_h);
  // Serial.printf("bounds: %d %d %d %d\n", str_x, str_y, str_w, str_h);
  // LECO_1976_Regular5pt7b -> 5 pixels width, 7 pixels height
  drawTextWithAlignment(&canvas, "20:42", 128 / 2, 7, HORIZONTAL_CENTER,
                        VERTICAL_MIDDLE);
  uint8_t systemBarHeight = 16;
  uint8_t menuItemHeight = 36;
  // canvas.fillRect(0, systemBarHeight, 128, menuItemHeight, themeColor);
  canvas.drawFastHLine(0, systemBarHeight - 1, 128, THREE_BIT_WHITE);
  canvas.drawXBitmap(127 - 20 - 4, 4, battery_charging_20x7, 20, 7,
                     THREE_BIT_WHITE);
  auto cursor_y = systemBarHeight + (menuItemHeight - 20) / 2;
  for (size_t i = 0; i < home_menu_items_count; i++) {
    bool selected = gSelectedItem == i;
    if (selected) {
      canvas.fillRoundRect(1, systemBarHeight + i * menuItemHeight, 128 - 2,
                           menuItemHeight, systemBarHeight / 2, themeColor);
    }
    uint16_t itemColor = selected ? THREE_BIT_WHITE : THREE_BIT_BLACK;
    MenuItem *item = home_menu_items + i;
    canvas.drawXBitmap(8, cursor_y, item->xbm_icon, item->icon_width,
                       item->icon_height, itemColor);
    canvas.setFont(&RasterGothic18CondBold9pt7b);
    canvas.setTextSize(1);
    canvas.setTextColor(itemColor);
    drawTextWithAlignment(&canvas, item->title, 128 / 2,
                          systemBarHeight + i * menuItemHeight +
                              menuItemHeight / 2,
                          HORIZONTAL_CENTER, VERTICAL_MIDDLE);
    cursor_y += menuItemHeight;
  }
  // // item 1
  // canvas.drawXBitmap(8, cursor_y, pref_20x20, 20, 20, THREE_BIT_WHITE);
  // canvas.setFont(&RasterGothic18CondBold9pt7b);
  // canvas.setTextSize(1);
  // canvas.setTextColor(THREE_BIT_WHITE);
  // drawTextWithAlignment(&canvas, "Settings", 128 / 2,
  //                       systemBarHeight + menuItemHeight - menuItemHeight /
  //                       2, HORIZONTAL_CENTER, VERTICAL_MIDDLE);
  // // item 2
  // cursor_y += menuItemHeight;
  // canvas.drawXBitmap(8, cursor_y, notifications_20x20, 20, 20,
  // THREE_BIT_BLACK); canvas.setTextColor(THREE_BIT_BLACK);
  // drawTextWithAlignment(&canvas, "Notifications", 128 / 2,
  //                       systemBarHeight + menuItemHeight / 2 +
  //                       menuItemHeight, HORIZONTAL_CENTER, VERTICAL_MIDDLE);
  // // item 3
  // cursor_y += menuItemHeight;
  // canvas.drawXBitmap(8, cursor_y, alarm_20x20, 20, 20, THREE_BIT_BLACK);
  // drawTextWithAlignment(&canvas, "Alarms", 128 / 2,
  //                       systemBarHeight + menuItemHeight / 2 +
  //                           menuItemHeight * 2,
  //                       HORIZONTAL_CENTER, VERTICAL_MIDDLE);

  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float real_vbat = calc_real_vbat(sensorValue);
  canvas.setCursor(80, 100);
  canvas.setTextColor(0b000);
  canvas.printf("%.4fV", real_vbat);

  canvas.refresh();
}

void show_macos_time() {
  canvas.clearDisplay();
  canvas.set3BitDrawPixelMode(true);
  canvas.drawRGBBitmap(0, 0, (uint16_t *)macos_128x128, 128, 128);
  canvas.set3BitDrawPixelMode(false);

  canvas.fillRect(3, 37, 107, 33, 0b100);
  canvas.setCursor(3, 37 + 25);
  canvas.setTextSize(1);
  // canvas.setFont(&FreeMono24pt7b);
  // canvas.setFont(&FreeMonoBold12pt7b);
  // canvas.setFont();
  canvas.setTextColor(0b111);
  char buf[16];
  DateTime now = rtc.now();
  snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%02d:%02d:%02d", now.hour(),
           now.minute(), now.second());
  canvas.println(buf);
  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float real_vbat = calc_real_vbat(sensorValue);
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
  snprintf(buf, sizeof(buf) / sizeof(buf[0]), "%02d:%02d:%02d", now.hour(),
           now.minute(), now.second());
  canvas.println(buf);
  auto sensorValue = analogRead(2);
  // Serial.println(sensorValue);
  float real_vbat = calc_real_vbat(sensorValue);
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
  canvas.print("0x");
  canvas.println(0xDEADBEEF, HEX);
  canvas.print("cost: ");
  canvas.println(last_cost, HEX);

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

unsigned long serial_cost;

void setup(void) {
  boot_time = millis();
  // btn_up_pressed_on_boot = !digitalRead(WAKEUP_PIN_UP);
  // btn_down_pressed_on_boot = !digitalRead(WAKEUP_PIN_DOWN);
  //  if (!ENABLE_DEEP_SLEEP) {
  auto serial_start_time = micros();
  Serial.begin(115200);
  serial_cost = micros() - serial_start_time;
  //  }
  // pinMode(SHARP_DISP, OUTPUT);
  // digitalWrite(SHARP_DISP, HIGH);
  SPI.begin(SHARP_SCK, 11, SHARP_MOSI, SHARP_SS);

  // start & clear the display
  canvas.begin();
  canvas.clearDisplay();
  // Several shapes are drawn centered on the screen.  Calculate 1/2 of
  // lesser of display width or height, this is used repeatedly later.
  minorHalfSize = min(canvas.width(), canvas.height()) / 2;
  Wire.begin(21, 20); // sda=  /scl=

#if defined(ENABLE_RTC)
  init_rtc();
#endif

  if (!ENABLE_DEEP_SLEEP) {
    show_datetime();
  }
  // show_anim();
  // show_time();
  // canvas.setRotation(0);
  // t_rex_setup();
  attachInterrupt(BTN_UP, btn_up_isr, FALLING);
  attachInterrupt(BTN_DOWN, btn_down_isr, FALLING);
  attachInterrupt(BTN_MID, btn_middle_isr, FALLING);
}

bool handle_button_events() {
  if (btn_up_pressed) {
    btn_up_pressed = false;
    Serial.println("btn_up pressed\n\n");
    gSelectedItem--;
    gSelectedItem %= home_menu_items_count;
    return true;
  }
  if (btn_down_pressed) {
    btn_down_pressed = false;
    Serial.println("btn_down pressed\n\n");
    gSelectedItem++;
    gSelectedItem %= home_menu_items_count;
    return true;
  }
  // TODO btn middle
  return false;
}

void loop(void) {
  // flappy_bird_loop();
  //  while (1) {
  //    t_rex_loop();
  //    delay(1);
  //  }
  // flappy_bird_loop();
  // show_macos_time();
  // show_macos_time_8_colors();
  // show_timeline_watch_face();
  // show_round_watch_face();
  // show_round_watch_face_pointer();
  // show_wheel_time();
  // show_random_ring_face(true);
  // show_home_menu();
  show_datetime_picker();
  if (ENABLE_DEEP_SLEEP) {
    for (int i = 0; i < 200; i++) {
      auto t = millis();
      while (millis() - t < 100) {
        if (handle_button_events()) {
          break;
        }
      }
      // show_home_menu();
      show_datetime_picker();
    }
    auto start_time = millis();
    show_random_ring_face(false);
    auto ring_face_cost = millis() - start_time;
    Serial.printf("face cost: %ldms\n", ring_face_cost);
    // esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(
        BIT(WAKEUP_PIN_UP) | BIT(WAKEUP_PIN_DOWN), DEFAULT_WAKEUP_LEVEL));
    Serial.printf("boot: %dms, serial: %dus\n", boot_time, serial_cost);
    Serial.println("--- NOW GOING TO SLEEP! ---");
    // esp_deep_sleep_enable_gpio_wakeup();
    esp_deep_sleep_start();
  } else {
    show_random_ring_face(true);
    Serial.println(last_cost);
    delay(900);
  }
}
