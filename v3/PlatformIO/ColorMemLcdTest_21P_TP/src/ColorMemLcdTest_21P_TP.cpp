
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <esp_sleep.h>

#include "ColorMemLCD.h"

#include <DS3231.h>

#include "IT7259Driver.h"
#include <Adafruit_INA219.h>
// #include "face1.h"
#include "calendar_rgb111.h"
#include "20150031.h"
#include "20150031_red.h"
#include "water_resist.h"
#include "water_resist_rgb111.h"

#include "pat9125.h"
#include <time.h>

#define PAT_POWER (47)
#define ENABLE_DEEP_SLEEP (0)

#define C3_COMPLETE_BOARD

#ifdef CONFIG_IDF_TARGET_ESP32C3
#define DEFAULT_WAKEUP_LEVEL ESP_GPIO_WAKEUP_GPIO_LOW
#endif

#define WAKEUP_TP_INT 0
#define WAKEUP_IMU_INT 1
#define WAKEUP_PIN_UP 4
// #define WAKEUP_PIN_DOWN 9

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* Time ESP32 will go to sleep (in seconds) */

// LED ANODE -> IO5

Adafruit_INA219 ina219;

RTClib myRTC;
DS3231 mySetRTC;


PAT9125 PAT(0x75);

// any pins can be used
#define SCK 17
#define MOSI 15
#define SS 18
#define EXTCOMIN -1

#define I2C1_SDA 3
#define I2C1_SCL 2

#define I2C2_SDA 41
#define I2C2_SCL 40

#define TP_INT 14
#define TP_RESET 21

#define SHARP_MISO -1

// 定义LEDC通道和GPIO
const int ledChannel = 0;
const int ledPin = 16;

// 定义LEDC参数
const int freq = 5000;                      // 频率（Hz）
const int resolution = 12;                  // 分辨率（位），ESP32支持1 - 13位
const int maxDuty = pow(2, resolution) - 1; // 最大占空比值

DateTime rtc_now;

void loop_rtc();

void testdrawtriangle();
void testfilltriangle();
void testdrawroundrect();
void testfillroundrect();
void testdrawrect();
void testdrawline();
void testdrawchar();
void testfillrect();

ColorMemLCD display(&SPI, SCK, MOSI, SS, EXTCOMIN, 7080000);

#define BLACK LCD_COLOR_BLACK
#define WHITE LCD_COLOR_WHITE

struct TouchPointData pointData;

void update_tp()
{
  readTouchEvent(&pointData);
  if (!pointData.isNull)
  {
    Serial.printf("x=%d,y=%d\n", pointData.xPos, pointData.yPos);
  }
}

void update_pat()
{
  PAT.pat9125_update();
  // Serial.printf("pat_x=%d\n", PAT.pat9125_x);
}

void IRAM_ATTR tp_int_isr()
{
  digitalRead(TP_INT) == HIGH ? Serial.println("H!") : Serial.println("L!");
}

void loop_ina219()
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  Serial.print("Bus Voltage:   ");
  Serial.print(busvoltage);
  Serial.println(" V");
  Serial.print("Shunt Voltage: ");
  Serial.print(shuntvoltage);
  Serial.println(" mV");
  Serial.print("Load Voltage:  ");
  Serial.print(loadvoltage);
  Serial.println(" V");
  Serial.print("Current:       ");
  Serial.print(current_mA);
  Serial.println(" mA");
  Serial.print("Power:         ");
  Serial.print(power_mW);
  Serial.println(" mW");
  Serial.println("");
}
void drawLineWithStrokeWidth(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t strokeWidth)
{
  // Calculate the direction vector of the line
  float dx = x1 - x0;
  float dy = y1 - y0;

  // Calculate the length of the line
  float length = sqrtf(dx * dx + dy * dy);

  // Normalize the direction vector
  float ux = dx / length;
  float uy = dy / length;

  // Calculate the normal (perpendicular) vector
  float nx = -uy;
  float ny = ux;

  // Scale the normal vector by half the stroke width
  float halfWidth = strokeWidth / 2.0f;
  float offsetX = nx * halfWidth;
  float offsetY = ny * halfWidth;

  // Calculate the four corners of the rectangle
  int16_t x0_left = x0 + offsetX;
  int16_t y0_left = y0 + offsetY;
  int16_t x0_right = x0 - offsetX;
  int16_t y0_right = y0 - offsetY;

  int16_t x1_left = x1 + offsetX;
  int16_t y1_left = y1 + offsetY;
  int16_t x1_right = x1 - offsetX;
  int16_t y1_right = y1 - offsetY;

  // Draw the first triangle
  display.fillTriangle(x0_left, y0_left,
                       x0_right, y0_right,
                       x1_left, y1_left,
                       color);

  // Draw the second triangle
  display.fillTriangle(x1_left, y1_left,
                       x0_right, y0_right,
                       x1_right, y1_right,
                       color);
}

std::vector<Point> polygon1 = {
    {15, 15},
    {176, 0},
    {176, 176},
    {0, 176},
};

std::vector<Point> polygon2 = {
    {58, 58},
    {132, 44},
    {176, 176},
    {15, 176 - 15},
};

std::vector<Point> polygon3 = {
    {176 / 2 - 5, 176 / 2},
    {176 / 2 + 15, 176 / 2 - 8},
    {176 - 8, 176 - 8},
    {15, 176 - 15},
};

std::vector<Point> polygon4 = {
    {176 / 2, 176 / 2},
    {176 - 30, 176 - 30},
    {176 / 3, 176 * 2 / 3},
};

std::vector<Point> polygon5 = {
    {176 / 2, 176 / 2},
    {176 * 2 / 3, 176 * 2 / 3},
    {176 / 2 - 10, 176 / 2 + 10},
};

const int NUM_POLYGONS = 5;
const int NUM_TARGET_POLYGONS = 4;

std::vector<Point> targetPolygon1 = {
    {176 / 2, 176 / 2},
    {176 * 3 / 4, 176 - 20},
    {176 / 2 - 5, 176 / 2 + 10},
};

std::vector<Point> targetPolygon2 = {
    {176 / 2, 176 / 2},
    {176 * 3 / 4, 176 - 20},
    {176 / 5, 176 - 5},
};

std::vector<Point> targetPolygon3 = {
    {176 / 2, 176 / 2},
    {176 * 3 / 4, 176 - 20},
    {176 / 5, 176 - 5},
    {15, 35},
};

std::vector<Point> targetPolygon4 = {
    {176 * 3 / 4, 176 - 20},
    {176 / 5, 176 - 5},
    {15, 35},
    {176 - 15, 35},
};

std::vector<Point> *polygons[] = {&polygon1, &polygon2, &polygon3, &polygon4, &polygon5};
std::vector<Point> *targetPolygons[] = {&targetPolygon1, &targetPolygon2, &targetPolygon3, &targetPolygon4};

uint8_t masks[NUM_POLYGONS][LCD_DISP_HEIGHT * MASK_BYTES_PER_ROW];
uint8_t targetMasks[NUM_TARGET_POLYGONS][LCD_DISP_HEIGHT * MASK_BYTES_PER_ROW];

void precomputeAllMasks()
{
  for (int i = 0; i < NUM_POLYGONS; ++i)
  {
    display.precomputeMask(*polygons[i], masks[i]);
  }
  for (int i = 0; i < NUM_TARGET_POLYGONS; ++i)
  {
    display.precomputeMask(*targetPolygons[i], targetMasks[i]);
  }
}

void drawPolygon(const std::vector<Point> &polygon, uint16_t color, uint8_t strokeWidth)
{
  for (size_t i = 0; i < polygon.size(); ++i)
  {
    Point p1 = polygon[i];
    Point p2 = polygon[(i + 1) % polygon.size()];
    drawLineWithStrokeWidth(p1.x, p1.y, p2.x, p2.y, color, strokeWidth);
    display.fillCircle(p1.x, p1.y, strokeWidth / 2, color);
    // drawLine(p1.x, p1.y, p2.x, p2.y);
  }
}

void waitForEnter()
{
  return;
  while (true)
  {
    if (Serial.available() > 0)
    {
      char inChar = Serial.read();
      if (inChar == '\n')
      {
        // 只检测回车符
        break; // 退出循环，继续执行程序
      }
    }
  }
}

void setup(void)
{
  pinMode(PAT_POWER, OUTPUT);
  digitalWrite(PAT_POWER, HIGH);
  Serial.begin(115200);
  // Serial.println("Hello Serial!");
  SPI.begin(SCK, SHARP_MISO, MOSI, SS);
  precomputeAllMasks();
  // SPI.setFrequency(8000000); // 8Mhz 容易花屏
  ledcSetup(ledChannel, freq, resolution);

  // 将LEDC通道绑定到GPIO引脚
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(ledChannel, maxDuty);

  pinMode(TP_INT, INPUT);
  pinMode(WAKEUP_PIN_UP, INPUT);
  // attachInterrupt(TP_INT, tp_int_isr, CHANGE);
  // tp i2c
  Wire.begin(I2C1_SDA, I2C1_SCL); // sda= /scl=
  if (!ENABLE_DEEP_SLEEP) {
    Wire1.begin(I2C2_SDA, I2C2_SCL);
  }
  // Serial.println("Hello!");

  // PAT.pat9125_init();
  // PAT.pat9125_set_res(240, 240, true);

  pinMode(TP_RESET, OUTPUT);
  digitalWrite(TP_RESET, HIGH);

  // start & clear the display

  loop_rtc();

  // Serial.print(rtc_now.year(), DEC);
  // Serial.print('/');
  // Serial.print(rtc_now.month(), DEC);
  // Serial.print('/');
  // Serial.print(rtc_now.day(), DEC);
  // Serial.print(' ');
  // Serial.print(rtc_now.hour(), DEC);
  // Serial.print(':');
  // Serial.print(rtc_now.minute(), DEC);
  // Serial.println();

  display.begin();

  // TODO: 这句还是需要的 display.clearDisplay();
  display.clearDisplay();
  display.fillScreen(LCD_COLOR_WHITE);
  display.setTextSize(3);
  display.setTextColor(LCD_COLOR_RED);

  char time_buf[] = "Time\n20241123\n\20:50:50\nCost\n1000ms\n\n\n\n\n\n\n\n";
  snprintf(time_buf, sizeof(time_buf), "Time\n%04d%02d%02d\n%02d:%02d:%02d\nCost\n%ldms\n",
           rtc_now.year(), rtc_now.month(), rtc_now.day(),
           rtc_now.hour(), rtc_now.minute(), rtc_now.second(),
           millis());

  display.fillScreen(LCD_COLOR_WHITE);
  display.setCursor(0, 10);
  display.println(time_buf);
  display.refresh();

  for (int i = 0; i < 1; i++)
  {
    testdrawtriangle();
    display.refresh();
    testdrawchar();
    display.refresh();
  }

  // if (!ina219.begin(&Wire1))
  // {
  //   Serial.println("Failed to find INA219 chip");
  // }
  // else
  // {
  //   Serial.println("Found INA219 chip");
  //   loop_ina219();
  //   ina219.powerSave(true);
  //   Serial.println("INA219 chip enter powerSave");
  // }

  display.setTextSize(3);
  display.setTextColor(LCD_COLOR_MAGENTA);
  display.setCursor(0, 128);
  display.println("Now SLEEP..");
  display.refresh();
  if (!ENABLE_DEEP_SLEEP)
  {
    for (int i = 0; i < 8000000; i++)
    {
      display.drawRGB111_4Bit(water_resist_rgb111);
      display.refresh();
      delay(500);
      // display.fillRoundRect(0, 0, 176, 176, 10, LCD_COLOR_BLACK);
      display.drawRGB111_4Bit(water_resist_rgb111);
      // display.drawRGBBitmap(0, 0, water_resist_rgb111, 176, 176);
      display.refresh();
      waitForEnter();
      for (uint8_t i = 0; i < NUM_POLYGONS; i++)
      {
        auto t = micros();
        display.setClipMask(masks[i]);
        display.clipOutPath(true);
        auto t1 = micros();
        // display.fillRoundRect(0, 0, 176, 176, 10, LCD_COLOR_YELLOW);
        display.drawRGB111_4Bit(water_resist_rgb111);
        auto t2 = micros();
        Serial.printf("frame %d: t1 %ldus, t2 %ldus\n", i, t1 - t, t2 - t1);
        display.clipOutPath(false);
        // display.fillRoundRect(0, 0, 176, 176, 10, LCD_COLOR_BLACK);
        display.fillScreen(LCD_COLOR_WHITE);
        display.clearClipMask();
        drawPolygon(*polygons[i], LCD_COLOR_BLACK, 9);
        display.refresh();
        Serial.printf("cost %ldus\n", micros() - t);
        waitForEnter();
      }
      display.fillScreen(LCD_COLOR_WHITE);
      display.refresh();
      waitForEnter();
      for (int8_t i = 0; i < NUM_TARGET_POLYGONS; i++)
      {
        auto t = micros();
        display.setClipMask(targetMasks[i]);
        display.clipOutPath(true);
        auto t1 = micros();
        display.drawRGB111_4Bit(calendar_rgb111);
        auto t2 = micros();
        Serial.printf("frame %d: t1 %ldus, t2 %ldus\n", i, t1 - t, t2 - t1);
        display.clipOutPath(false);
        // display.fillRoundRect(0, 0, 176, 176, 10, LCD_COLOR_BLACK);
        display.fillScreen(LCD_COLOR_WHITE);
        display.clearClipMask();
        drawPolygon(*targetPolygons[i], LCD_COLOR_BLACK, 9);
        display.refresh();
        Serial.printf("cost %ldus\n", micros() - t);
        waitForEnter();
      }
      display.drawRGB111_4Bit(calendar_rgb111);
      display.refresh();
      waitForEnter();

      // display.refresh();
      // delay(500);

      // display.fillRoundRect(0, 0, 176, 176, 10, LCD_COLOR_YELLOW);
      delay(500);
    }
  }

  // TODO eggfly mod
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
  display.print("0x");
  display.println(0xDEADBEEF, HEX);
  display.refresh();
  delay(1000);
}

// uint16_t colors[] = {LCD_COLOR_MAGENTA, };
// uint8_t color_count = sizeof(colors) / sizeof(colors[0]);

uint16_t color = 0;

size_t fadeTimes = 0;
size_t frame_count = 0;

void fadeOutAndIn()
{
  // 减少亮度（淡出）
  for (int duty = maxDuty; duty >= 0; duty--)
  {
    ledcWrite(ledChannel, duty);
    delayMicroseconds(200); // 控制淡出速度
  }
  // 增加亮度（淡入）
  for (int duty = 0; duty <= maxDuty; duty++)
  {
    ledcWrite(ledChannel, duty);
    delayMicroseconds(200); // 控制淡入速度
  }
}

long draw_cost;
long refresh_cost;


// time_t unix_time(DateTime& t) {
//   char buf[100];
//   strncpy(buf, "YYYY-MM-DD hh:mm:ss\0", sizeof(buf));
//   t.format(buf);
//   struct tm tm;
//   time_t epoch;
//   if (strptime(buf, "%Y-%m-%d %H:%M:%S", &tm) != NULL ) {
//     epoch = mktime(&tm);
// #ifdef APP_DEBUG
//     Serial.print("unix timestamp value: ");
//     Serial.println(epoch);
// #endif
//     return epoch;
//   } else {
//     Serial.println("time convert error");
//     return 0;
//   }
// }

void loop_rtc()
{
  rtc_now = myRTC.now();
  DateTime compileTime = DateTime(__DATE__, __TIME__);
  if (rtc_now.year() < compileTime.year() - 1)
  {
    Serial.println("RTC is older than compile time! Updating RTC...");
    mySetRTC.setEpoch(compileTime.unixtime());
    rtc_now = myRTC.now();
  }

  // Serial.print(rtc_now.year(), DEC);
  // Serial.print('/');
  // Serial.print(rtc_now.month(), DEC);
  // Serial.print('/');
  // Serial.print(rtc_now.day(), DEC);
  // Serial.print(' ');
  // Serial.print(rtc_now.hour(), DEC);
  // Serial.print(':');
  // Serial.print(rtc_now.minute(), DEC);
  // Serial.println();

  // Serial.print(':');
  // Serial.print(now.second(), DEC);
  // Serial.println();

  // Serial.print(" since midnight 1/1/1970 = ");
  // Serial.print(now.unixtime());
  // Serial.print("s = ");
  // Serial.print(now.unixtime() / 86400L);
  // Serial.println("d");
}

void loop(void)
{
  auto t = micros();
  if (ENABLE_DEEP_SLEEP)
  {
    // Serial.println("set reset tp to low");
    // digitalWrite(TP_RESET, LOW); // 注释掉这行，会反复重启；加上这行，触摸不会唤醒
    // digitalWrite(TP_RESET, HIGH); // 注释掉这行，会反复重启；加上这行，触摸不会唤醒
    // delay(500);
    // Serial.println("sleep 500ms ok.");
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.printf("--- NOW SLEEP! --- %ldms\n", t);
    esp_deep_sleep_start();
  }
  frame_count++;
  if (frame_count == 20)
  {
    frame_count = 0;
    fadeTimes = 3;
  }
  // Uncomment the following line to enable fading
  // while (fadeTimes > 0)
  // {
  //   fadeOutAndIn();
  //   fadeTimes--;
  // }

  // Screen must be refreshed at least once per second
  auto x = random(0, 175);
  auto y = random(0, 175);

  display.fillCircle(x, y, 32, color << 1);
  //  Serial.println(color << 1);
  //  Serial.println(LCD_COLOR_RED);

  color++;
  color %= 8;
  display.fillRect(5, 5, 176 - 5 * 2, 45, LCD_COLOR_WHITE);
  display.fillRect(176 - 10, 0, 10, 176, LCD_COLOR_YELLOW);
  for (int i = 0; i < 1; i++)
  {
    update_tp();
    if (!pointData.isNull)
    {
      display.setTextSize(2);
      display.setTextColor(LCD_COLOR_RED);
      display.setCursor(10, 10);
      display.printf("x=%d,y=%d\n", pointData.xPos, pointData.yPos);
      display.drawFastVLine(pointData.xPos, 0, 176, LCD_COLOR_BLACK);
      display.drawFastHLine(0, pointData.yPos, 176, LCD_COLOR_BLACK);
      break;
    }
  }
  // update_pat();
  display.setTextSize(2);
  display.setTextColor(LCD_COLOR_RED);

  // digital crown value
  display.setTextSize(2);
  display.setCursor(10, 10);
  display.setTextColor(LCD_COLOR_BLACK);
  display.printf("Crown=%ld", PAT.pat9125_x);
  // draw scoll bar
  int16_t bar_pos = constrain(map(PAT.pat9125_x, 0, 176 * 2, 176, 0), 0, 176);
  display.fillRoundRect(176 - 10 + 1, bar_pos, 10 - 2, 20, 3, LCD_COLOR_BLACK);

  display.setTextColor(LCD_COLOR_MAGENTA);
  display.setTextSize(2);
  display.setCursor(10, 30);
  display.printf("%ldms %ldms", draw_cost, refresh_cost);
  draw_cost = micros() - t;
  display.refresh();
  // delay(10);
  refresh_cost = micros() - t - draw_cost;
  Serial.printf("draw cost %ldus, refresh cost %ldus\n", draw_cost, refresh_cost);
  // delay(100);
}

void testdrawchar(void)
{
  display.setTextSize(1);
  display.setTextColor(LCD_COLOR_BLUE);
  display.setCursor(0, 0);

  for (uint8_t i = 0; i < 168; i++)
  {
    if (i == '\n')
      continue;
    display.write(i);
    // if ((i > 0) && (i % 14 == 0))
    // display.println();
  }
  display.refresh();
}

void testdrawcircle(void)
{
  for (uint8_t i = 0; i < display.height(); i += 2)
  {
    display.drawCircle(display.width() / 2 - 5, display.height() / 2 - 5, i, BLACK);
    display.refresh();
  }
}

void testfillrect(void)
{
  uint8_t color = 1;
  for (uint8_t i = 0; i < display.height() / 2; i += 3)
  {
    // alternate colors
    display.fillRect(i, i, display.width() - i * 2, display.height() - i * 2, color % 2);
    display.refresh();
    color++;
  }
}

void testdrawtriangle(void)
{
  for (uint16_t i = 0; i < min(display.width(), display.height()) / 2; i += 5)
  {
    display.drawTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, BLACK);
    display.refresh();
  }
}

void testfilltriangle(void)
{
  uint8_t color = BLACK;
  for (int16_t i = min(display.width(), display.height()) / 2; i > 0; i -= 5)
  {
    display.fillTriangle(display.width() / 2, display.height() / 2 - i,
                         display.width() / 2 - i, display.height() / 2 + i,
                         display.width() / 2 + i, display.height() / 2 + i, color);
    if (color == WHITE)
      color = BLACK;
    else
      color = WHITE;
    display.refresh();
  }
}

void testdrawroundrect(void)
{
  for (uint8_t i = 0; i < display.height() / 4; i += 2)
  {
    display.drawRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, BLACK);
    display.refresh();
  }
}

void testfillroundrect(void)
{
  uint8_t color = BLACK;
  for (uint8_t i = 0; i < display.height() / 4; i += 2)
  {
    display.fillRoundRect(i, i, display.width() - 2 * i, display.height() - 2 * i, display.height() / 4, color);
    if (color == WHITE)
      color = BLACK;
    else
      color = WHITE;
    display.refresh();
  }
}

void testdrawrect(void)
{
  for (uint8_t i = 0; i < display.height() / 2; i += 2)
  {
    display.drawRect(i, i, display.width() - 2 * i, display.height() - 2 * i, BLACK);
    display.refresh();
  }
}

void testdrawline()
{
  // display.setTextColor(LCD_COLOR_RED);
  for (uint8_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, 0, i, display.height() - 1, LCD_COLOR_RED);
    display.refresh();
  }
  // ;
  // display.setTextColor(LCD_COLOR_MAGENTA);
  for (uint8_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(0, 0, display.width() - 1, i, LCD_COLOR_MAGENTA);
    display.refresh();
  }
  delay(150);
  // return;

  display.clearDisplay();
  for (uint8_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(0, display.height() - 1, i, 0, BLACK);
    display.refresh();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(0, display.height() - 1, display.width() - 1, i, LCD_COLOR_YELLOW);
    display.refresh();
  }
  delay(150);

  display.clearDisplay();
  for (int8_t i = display.width() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, i, 0, LCD_COLOR_GREEN);
    display.refresh();
  }
  for (int8_t i = display.height() - 1; i >= 0; i -= 4)
  {
    display.drawLine(display.width() - 1, display.height() - 1, 0, i, BLACK);
    display.refresh();
  }
  delay(150);

  display.clearDisplay();
  for (uint8_t i = 0; i < display.height(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, 0, i, BLACK);
    display.refresh();
  }
  for (uint8_t i = 0; i < display.width(); i += 4)
  {
    display.drawLine(display.width() - 1, 0, i, display.height() - 1, BLACK);
    display.refresh();
  }
  delay(150);
}
