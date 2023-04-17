
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <ColorMemLCD.h>

#include "IT7259Driver.h"

// any pins can be used
#define SCK 6
#define MOSI 7
#define SS 8
#define EXTCOMIN 11
#define DISP 10

#define SHARP_MISO -1

ColorMemLCD display(SCK, MOSI, SS, EXTCOMIN);

#define BLACK LCD_COLOR_BLACK
#define WHITE LCD_COLOR_WHITE

struct TouchPointData pointData;

void update_tp() {
  readTouchEvent(&pointData);
  if (!pointData.isNull) {
    Serial.printf("x=%d,y=%d\n", pointData.xPos, pointData.yPos);
  }
}

void IRAM_ATTR isr() {
  Serial.println("!");
}


void setup(void) {
  Serial.begin(115200);
  SPI.begin(SCK, SHARP_MISO, MOSI, SS);
  SPI.setFrequency(6000000);

  pinMode(9, INPUT_PULLUP);
  attachInterrupt(9, isr, CHANGE);

  Serial.println("Hello!");

  // tp i2c
  Wire.begin(2, 3);   // sda= /scl=

  pinMode(DISP, OUTPUT);
  digitalWrite(DISP, HIGH);

  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);

  // start & clear the display
  display.begin();
  display.clearDisplay();

  return;

  // draw a single pixel
  display.drawPixel(10, 10, BLACK);
  display.refresh();
  delay(500);
  display.clearDisplay();

  // draw the first ~12 characters in the font
  testdrawchar();
  display.refresh();
  delay(500);
  display.clearDisplay();


  display.refresh();
  // draw many lines
  testdrawline();
  delay(100);
  display.clearDisplay();

  // draw rectangles
  testdrawrect();
  delay(100);
  display.clearDisplay();

  // draw multiple rectangles
  testfillrect();
  display.refresh();
  delay(100);
  display.clearDisplay();

  // draw a circle, 10 pixel radius
  display.fillCircle(display.width() / 2, display.height() / 2, 10, BLACK);
  display.refresh();
  delay(100);
  display.clearDisplay();

  testdrawroundrect();
  display.refresh();
  delay(100);
  display.clearDisplay();

  testfillroundrect();
  display.refresh();
  delay(100);
  display.clearDisplay();

  testdrawtriangle();
  display.refresh();
  delay(100);
  display.clearDisplay();

  testfilltriangle();
  display.refresh();
  delay(100);
  display.clearDisplay();

  // draw the first ~12 characters in the font
  testdrawchar();
  display.refresh();
  delay(100);
  display.clearDisplay();

  // text display tests
  display.setTextSize(1);
  display.setTextColor(LCD_COLOR_MAGENTA);
  display.setCursor(0, 0);
  display.println("Hello, world!");
  display.setTextColor(LCD_COLOR_WHITE, LCD_COLOR_BLACK); // 'inverted' text
  display.println(3.141592);
  display.setTextSize(2);
  display.setTextColor(LCD_COLOR_YELLOW);
  display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.refresh();
  delay(1000);
}

// uint16_t colors[] = {LCD_COLOR_MAGENTA, };
// uint8_t color_count = sizeof(colors) / sizeof(colors[0]);

uint16_t color = 0;

void loop(void) {
  // Screen must be refreshed at least once per second
  auto x = random(0, 175);
  auto y = random(0, 175);

  display.fillCircle(x, y, 32, color << 1);
  //  Serial.println(color << 1);
  //  Serial.println(LCD_COLOR_RED);

  color++;
  color %= 8;
  display.fillRect(10, 9, 130, 20, LCD_COLOR_WHITE);
  for (int i = 0; i < 1; i++) {
    update_tp();
    if (!pointData.isNull) {
      display.setTextSize(2);
      display.setTextColor(LCD_COLOR_RED);
      display.setCursor(10, 10);
      display.printf("x=%d,y=%d\n", pointData.xPos, pointData.yPos);
      display.drawFastVLine(pointData.xPos, 0, 176, LCD_COLOR_BLACK);
      display.drawFastHLine(0, pointData.yPos, 176, LCD_COLOR_BLACK);
      break;
    }
  }
  display.refresh();
  // delay(1);
}

void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(LCD_COLOR_BLUE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    //if ((i > 0) && (i % 14 == 0))
    //display.println();
  }
  display.refresh();
}

void testdrawcircle(void) {
  for (uint8_t i = 0; i < display.height(); i += 2) {
    display.drawCircle(display.width() / 2 - 5, display.height() / 2 - 5, i, BLACK);
    display.refresh();
  }
}

void testfillrect(void) {
  uint8_t color = 1;
  for (uint8_t i = 0; i < display.height() / 2; i += 3) {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.refresh();
    color++;
  }
}

void testdrawtriangle(void) {
  for (uint16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5) {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, BLACK);
    display.refresh();
  }
}

void testfilltriangle(void) {
  uint8_t color = BLACK;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5) {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.refresh();
  }
}

void testdrawroundrect(void) {
  for (uint8_t i = 0; i < display.height() / 4; i += 2) {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, BLACK);
    display.refresh();
  }
}

void testfillroundrect(void) {
  uint8_t color = BLACK;
  for (uint8_t i = 0; i < display.height() / 4; i += 2) {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, color);
    if (color == WHITE) color = BLACK;
    else color = WHITE;
    display.refresh();
  }
}

void testdrawrect(void) {
  for (uint8_t i = 0; i < display.height() / 2; i += 2) {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, BLACK);
    display.refresh();
  }
}

void testdrawline() {
  //display.setTextColor(LCD_COLOR_RED);
  for (uint8_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, 0, i, display.height() - 1, LCD_COLOR_RED);
    display.refresh();
  }
  // ;
  //display.setTextColor(LCD_COLOR_MAGENTA);
  for (uint8_t i = 0; i < display.height(); i += 4) {
    display.drawLine(0, 0, display.width() - 1, i, LCD_COLOR_MAGENTA);
    display.refresh();
  }
  delay(150);
  // return;

  display.clearDisplay();
  for (uint8_t i = 0; i < display.width(); i += 4) {
    display.drawLine(0, display.height() - 1, i, 0, BLACK);
    display.refresh();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, LCD_COLOR_YELLOW);
    display.refresh();
  }
  delay(150);

  display.clearDisplay();
  for (int8_t i = display.width() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, LCD_COLOR_GREEN);
    display.refresh();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4) {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, BLACK);
    display.refresh();
  }
  delay(150);

  display.clearDisplay();
  for (uint8_t i = 0; i < display.height(); i += 4) {
    display.drawLine(display.width() - 1, 0, 0, i, BLACK);
    display.refresh();
  }
  for (uint8_t i = 0; i < display.width(); i += 4) {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, BLACK);
    display.refresh();
  }
  delay(150);
}
