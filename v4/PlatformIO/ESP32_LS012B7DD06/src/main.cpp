#include <Arduino.h>
#include "color_pad.h"
#include "mario.h"

#include "ParallelColorMemLCD.h"

// 效果还是 LEDC 好一些
#define USE_2ND_CORE_PWM (1)

#define FREQ_VCOM 1
#define BL_FREQ 120

const int ledChannel = 0; // LEDC 通道
const int resolution = 8; // 分辨率（8 位）

const int BACKLIGHT_LEDC_CHANNEL = 3;
// Pin definitions
#define XRST_PIN 25
#define VST_PIN 0
#define VCK_PIN 5
#define ENB_PIN 26
#define HST_PIN 4
#define HCK_PIN 21
#define R1_PIN 12
#define R2_PIN 13
#define G1_PIN 14
#define G2_PIN 15
#define B1_PIN 32
#define B2_PIN 33
#define VCOM_PIN 27
// VCOM same as FRP
#define FRP_PIN 22
// VA = XFRP
#define XFRP_PIN 19
#define LED_PIN 2

#define BACKLIGHT_PIN 23

#define COLOR_BLACK 0b00000000      // 黑色
#define COLOR_WHITE 0b00111111      // 白色
#define COLOR_RED 0b00110000        // 红色
#define COLOR_GREEN 0b00001100      // 绿色
#define COLOR_BLUE 0b00000011       // 蓝色
#define COLOR_CYAN 0b00001111       // 青色
#define COLOR_MAGENTA 0b00110011    // 品红
#define COLOR_YELLOW 0b00111100     // 黄色
#define COLOR_GRAY 0b00110000       // 灰色
#define COLOR_DARK_RED 0b00100000   // 深红
#define COLOR_DARK_GREEN 0b00001000 // 深绿
#define COLOR_DARK_BLUE 0b00000010  // 深蓝
#define COLOR_ORANGE 0b00110100     // 橙色
#define COLOR_PURPLE 0b00100100     // 紫色
#define COLOR_BROWN 0b00100010      // 棕色
#define COLOR_LIGHT_GRAY 0b00111000 // 浅灰色

uint8_t _16_colors[16] = {
    COLOR_BLACK,
    COLOR_WHITE,
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE,
    COLOR_CYAN,
    COLOR_MAGENTA,
    COLOR_YELLOW,
    COLOR_GRAY,
    COLOR_DARK_RED,
    COLOR_DARK_GREEN,
    COLOR_DARK_BLUE,
    COLOR_ORANGE,
    COLOR_PURPLE,
    COLOR_BROWN,
    COLOR_LIGHT_GRAY};

// ESP32 GPIO:
// 1: TX0 PIN
// 3: RX0 PIN
// 6 - 11 CANNOT USE, connected to the integrated SPI flash
// 34 - 39 INPUT ONLY
void resetDisplay()
{
  digitalWrite(XRST_PIN, HIGH);
  // delayMicroseconds(500);
  digitalWrite(XRST_PIN, LOW);
  // delayMicroseconds(23000);
  digitalWrite(XRST_PIN, HIGH);
}

void initDisplay()
{
  digitalWrite(VCK_PIN, LOW);
  digitalWrite(VST_PIN, LOW);
  digitalWrite(HST_PIN, LOW);
  digitalWrite(HCK_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
}

// put function definitions here:
void anotherTask(void *parameter)
{
  pinMode(VCOM_PIN, OUTPUT);
  pinMode(FRP_PIN, OUTPUT);
  pinMode(XFRP_PIN, OUTPUT);
  size_t loopCount = 0;
  while (true)
  {
    // digitalWrite(VCOM_PIN, HIGH);
    // digitalWrite(FRP_PIN, HIGH);
    // digitalWrite(XFRP_PIN, LOW);
    // delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    digitalWrite(VCOM_PIN, LOW);
    digitalWrite(FRP_PIN, LOW);
    digitalWrite(XFRP_PIN, HIGH);
    delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    while (1)
    {
      delay(1000);
      Serial.printf("anotherTask %d\n", loopCount);
    }
    loopCount++;
  }
}

// State variables for toggling
bool hckState = LOW;
bool vckState = LOW;

// Function to flush the framebuffer to the display
void flushDisplay()
{
  digitalWrite(VST_PIN, HIGH);
  // delayMicroseconds(20000);
  vckState = !vckState;
  digitalWrite(VCK_PIN, vckState);

  for (int i = 1; i <= 487; i++)
  {
    if (i == 1)
    {
      digitalWrite(VST_PIN, LOW);
      // delayMicroseconds(21000);
    }

    if (i >= 2 && i <= 481)
    {
      digitalWrite(HST_PIN, HIGH);
      hckState = !hckState; // Toggle HCK state
      digitalWrite(HCK_PIN, hckState);
      // delayMicroseconds(1000);
      // 121 -> 123 了
      // 夏普需要123，并且一定是123
      // JDI 是 121
      for (int j = 1; j <= 123; j++)
      {
        if (j == 1)
        {
          digitalWrite(HST_PIN, LOW);
        }
        // 夏普屏在最后 ENB，JDI 在 20个 CLOCK的时候做 ENB
        if (j == 122)
        {
          digitalWrite(ENB_PIN, HIGH);
        }
        else if (j == 123)
        {
          digitalWrite(ENB_PIN, LOW);
        }

        if (j >= 2 && j <= 121)
        {
          int y = (i - 2) / 2;
          int x = j - 2;
          int pos = (x * 2) + 240 * y;

          // Send pixel data
          uint8_t pixel = canvas->getBuffer()[pos];
          // 这里 JDI 和 夏普屏都需要有 LPB/SPB 的区别，所以必须有下面的 if/else
          if (i % 2 == 0)
          {
            // LPB
            digitalWrite(R1_PIN, (pixel & 0b100000) ? HIGH : LOW);
            digitalWrite(G1_PIN, (pixel & 0b001000) ? HIGH : LOW);
            digitalWrite(B1_PIN, (pixel & 0b000010) ? HIGH : LOW);
            pixel = canvas->getBuffer()[pos + 1];
            digitalWrite(R2_PIN, (pixel & 0b100000) ? HIGH : LOW);
            digitalWrite(G2_PIN, (pixel & 0b001000) ? HIGH : LOW);
            digitalWrite(B2_PIN, (pixel & 0b000010) ? HIGH : LOW);
          }
          else
          {
            // SPB
            digitalWrite(R1_PIN, (pixel & 0b010000) ? HIGH : LOW);
            digitalWrite(G1_PIN, (pixel & 0b000100) ? HIGH : LOW);
            digitalWrite(B1_PIN, (pixel & 0b000001) ? HIGH : LOW);
            pixel = canvas->getBuffer()[pos + 1];
            digitalWrite(R2_PIN, (pixel & 0b010000) ? HIGH : LOW);
            digitalWrite(G2_PIN, (pixel & 0b000100) ? HIGH : LOW);
            digitalWrite(B2_PIN, (pixel & 0b000001) ? HIGH : LOW);
          }
        }

        // Toggle HCK state
        // TODO
        // delayMicroseconds(500);
        hckState = !hckState;
        digitalWrite(HCK_PIN, hckState);
      }
    }
    else
    {
      // delayMicroseconds(1000);
    }

    // delayMicroseconds(500);
    // Toggle VCK state
    vckState = !vckState;
    digitalWrite(VCK_PIN, vckState);
  }
}

uint8_t colors[] = {
    0b010000,
    0b100000,
    0b110000,
    0b000100,
    0b001000,
    0b001100,
    0b000001,
    0b000010,
    0b000011,
};
size_t currColorIndex = 0;
size_t maxColorIndex = sizeof(colors) / sizeof(colors[0]);

void testFillCircle()
{
  auto x = random(0, LCD_DISP_WIDTH);
  auto y = random(0, LCD_DISP_HEIGHT);
  uint8_t color = random(0, 64);
  canvas->fillCircle(x, y, 32, color);
}

size_t current16ColorsIndex = 0;

void testFillRainbow()
{
  // canvas->fillCircle
  for (int i = 0; i < 16; i++)
  {
    canvas->fillCircle(120, 120, 120 - i * 7, _16_colors[i]);
  }
  // canvas->fillScreen(_16_colors[current16ColorsIndex]);
  current16ColorsIndex++;
  current16ColorsIndex = current16ColorsIndex % 16;
  Serial.printf("current16ColorsIndex: %d\n", current16ColorsIndex);
}

void drawInitialContent()
{
  // Example: Draw "Hello World" and some lines
  // This function should implement the drawing logic similar to the Rust code
  // For simplicity, we will just fill the frame buffer with a color

  // Fill the frame buffer with a color (e.g., white)
  for (int y = 0; y < 240; y++)
  {
    for (int x = 0; x < 240; x++)
    {
      int index = (y * 240 + x); // 1 byte per pixel
      canvas->getBuffer()[index] = colors[currColorIndex];
    }
  }
  currColorIndex++;
  currColorIndex = currColorIndex % maxColorIndex;
  // Send the frame buffer to the display
  // flushDisplay();
}

void drawInitialContent2()
{
  canvas->fillScreen(colors[currColorIndex]);
  currColorIndex++;
  currColorIndex = currColorIndex % maxColorIndex;
  // canvas->drawCircle(120, 120, 16, 0b111111);
  // Send the frame buffer to the display
  // flushDisplay();
  canvas->fillCircle(120, 120, 16, 0b111111);
}

void initLedc()
{
  ledcSetup(ledChannel, FREQ_VCOM, resolution);
  ledcSetup(1, FREQ_VCOM, resolution);
  delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
  ledcSetup(2, FREQ_VCOM, resolution);
  // 将 LEDC 通道连接到 GPIO
  ledcAttachPin(VCOM_PIN, ledChannel);
  ledcAttachPin(FRP_PIN, 1);
  // delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
  ledcAttachPin(XFRP_PIN, 2);

  ledcWrite(ledChannel, 128); // 设置 PWM 占空比
  ledcWrite(1, 128);
  // 高低电平半个周期
  // delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
  ledcWrite(2, 128);
}

void setup()
{
  Serial.begin(115200);
  ledcSetup(BACKLIGHT_LEDC_CHANNEL, BL_FREQ, resolution);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_LEDC_CHANNEL);
  ledcWrite(BACKLIGHT_LEDC_CHANNEL, 220);
  canvas = new GFXcanvas8(240, 240);
  // delay(500);
  Serial.println("after new GFXcanvas8 and delay");
  if (USE_2ND_CORE_PWM)
  {
    int coreId = xPortGetCoreID();
    int anotherCoreId = 1 - coreId;
    Serial.printf("Core ID: %d\n", coreId);
    xTaskCreatePinnedToCore(
        anotherTask,    /* Function to implement the task */
        "anotherTask",  /* Name of the task */
        10000,          /* Stack size in words */
        NULL,           /* Task input parameter */
        0,              /* Priority of the task */
        NULL,           /* Task handle. */
        anotherCoreId); /* Core where the task should run */
  }
  else
  {
    initLedc();
  }

  pinMode(LED_PIN, OUTPUT);
  pinMode(XRST_PIN, OUTPUT);
  pinMode(VST_PIN, OUTPUT);
  pinMode(VCK_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(HST_PIN, OUTPUT);
  pinMode(HCK_PIN, OUTPUT);
  pinMode(R1_PIN, OUTPUT);
  pinMode(R2_PIN, OUTPUT);
  pinMode(G1_PIN, OUTPUT);
  pinMode(G2_PIN, OUTPUT);
  pinMode(B1_PIN, OUTPUT);
  pinMode(B2_PIN, OUTPUT);

  // Initialize the display
  resetDisplay();
  Serial.println("after resetDisplay");
  // delay(3000);
  initDisplay();
  Serial.println("after initDisplay");
  // delay(3000);
  // drawInitialContent2();
  // memcpy(canvas->getBuffer(), mario, 240 * 240);
  // memcpy(canvas->getBuffer(), images, 240 * 240);
}

size_t frame_index = 0;

void loop()
{
  Serial.println("before testFillCircle");
  // canvas->fillCircle(120, 120, 16, 0b110000);
  // testFillRainbow();
  // testFillCircle();
  //
  switch (frame_index)
  {
  case 0:
    testFillRainbow();
    break;
  case 1:
    canvas->fillScreen(0b010000);
    break;
  case 2:
    canvas->fillScreen(0b100000);
    break;
  case 3:
    canvas->fillScreen(0b110000);
    break;
  case 4:
    canvas->fillScreen(0b000100);
    break;
  case 5:
    canvas->fillScreen(0b001000);
    break;
  case 6:
    canvas->fillScreen(0b001100);
    break;
  case 7:
    canvas->fillScreen(0b000001);
    break;
  case 8:
    canvas->fillScreen(0b000010);
    break;
  case 9:
    canvas->fillScreen(0b000011);
    break;
  case 10:
    memcpy(canvas->getBuffer(), mario, 240 * 240);
    break;
  case 11:
    memcpy(canvas->getBuffer(), images, 240 * 240);
    break;
  default:
    break;
  }
  // canvas->fillCircle(120, 120, 3, 0b111111);
  Serial.println("after testFillCircle");
  Serial.println("before flushDisplay");
  auto t = micros();
  uint8_t x = 0; // 1就没了，0更没有
  uint8_t x2 = 239;
  canvas->drawLine(x, 0, x, 240, 0b111111);
  canvas->drawLine(x2, 0, x2, 240, 0b111111);
  flushDisplay();
  frame_index++;
  if (frame_index >= 12)
  {
    // while (1)
    delay(5000);
  }
  frame_index = frame_index % 12;
  delay(300);
  Serial.printf("flushDisplay took %d us\n", micros() - t);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  Serial.println("after LED");
}
