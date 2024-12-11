#include <Arduino.h>
#include "color_pad.h"
#include "mario.h"
#include "rainbow_frame_1.h"
#include "anim_data_12_frames.h"
#include "anim_data_7_frames.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include "esp_check.h"
#include "driver/dedic_gpio.h"
#include "driver/gpio.h"

#include "ParallelColorMemLCD.h"

#define USE_2ND_CORE_PWM (1)

#define USE_CONST_VCOM_NOT_PWM (1)

// USE: 19801 us per frame, NOT USE: 118875 us per frame
#define USE_S3_DEDICATED_GPIO (1)

#define FREQ_VCOM 180
#define BL_FREQ 120

const int ledChannel = 0;  // LEDC 通道
const int resolution = 12; // 分辨率（12 位）, 8位好像只有ESP32可以用

const int BACKLIGHT_LEDC_CHANNEL = 1;

// BL
#define BACKLIGHT_PIN 16

// Pin definitions
// GSP
#define VST_PIN 1
// GCK
#define VCK_PIN 2
// GEN
#define ENB_PIN 3
// INTB
#define XRST_PIN 4
// VCOM same as FRP = VB
#define FRP_PIN 5
// VA = XFRP
#define XFRP_PIN 6

// BSP
#define HST_PIN 7
// BCK
#define HCK_PIN 8

#define R1_PIN 9
#define R2_PIN 10
#define G1_PIN 11
#define G2_PIN 12
#define B1_PIN 13
#define B2_PIN 14

// VCOM
#define VCOM_PIN 15

// NO USE
#define LED_PIN 21

#define SLEEP_DURATION 100           // 深度睡眠时间（秒）  


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

void initScreenPWMPins()
{
  Serial.println("initScreenPWMPins");
  pinMode(VCOM_PIN, OUTPUT);
  pinMode(FRP_PIN, OUTPUT);
  pinMode(XFRP_PIN, OUTPUT);

  digitalWrite(VCOM_PIN, LOW);
  digitalWrite(FRP_PIN, LOW);
  digitalWrite(XFRP_PIN, HIGH);
}

// put function definitions here:
void anotherTask(void *parameter)
{
  initScreenPWMPins();
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
      // Serial.printf("anotherTask %d\n", loopCount);
    }
    loopCount++;
  }
}

// State variables for toggling
bool hckState = LOW;
bool vckState = LOW;

// static inline __attribute__((always_inline)) void directWriteLow(IO_REG_TYPE pin)
// {
//   if (pin < 32)
//     GPIO.out_w1tc = ((uint32_t)1 << pin);
//   else if (pin < 34)
//     GPIO.out1_w1tc.val = ((uint32_t)1 << (pin - 32));
// }

// static inline __attribute__((always_inline)) void directWriteHigh(IO_REG_TYPE pin)
// {
//   if (pin < 32)
//     GPIO.out_w1ts = ((uint32_t)1 << pin);
//   else if (pin < 34)
//     GPIO.out1_w1ts.val = ((uint32_t)1 << (pin - 32));
// }

#define directWriteLow(pin)                               \
  do                                                      \
  {                                                       \
    if ((pin) < 32)                                       \
      GPIO.out_w1tc = ((uint32_t)1 << (pin));             \
    else if ((pin) < 34)                                  \
      GPIO.out1_w1tc.val = ((uint32_t)1 << ((pin) - 32)); \
  } while (0)

#define directWriteHigh(pin)                              \
  do                                                      \
  {                                                       \
    if ((pin) < 32)                                       \
      GPIO.out_w1ts = ((uint32_t)1 << (pin));             \
    else if ((pin) < 34)                                  \
      GPIO.out1_w1ts.val = ((uint32_t)1 << ((pin) - 32)); \
  } while (0)

dedic_gpio_bundle_handle_t bundleA = NULL;

void dedic_gpio_new()
{
  const int bundleA_gpios[] = {
      B2_PIN,
      B1_PIN,
      G2_PIN,
      G1_PIN,
      R2_PIN,
      R1_PIN,
  };
  for (size_t i = 0; i < sizeof(bundleA_gpios) / sizeof(bundleA_gpios[0]); i++)
  {
    pinMode(bundleA_gpios[i], OUTPUT);
  }
  gpio_config_t io_conf = {
      .mode = GPIO_MODE_OUTPUT,
  };
  for (int i = 0; i < sizeof(bundleA_gpios) / sizeof(bundleA_gpios[0]); i++)
  {
    io_conf.pin_bit_mask = 1ULL << bundleA_gpios[i];
    gpio_config(&io_conf);
  }
  // 创建 bundleA，仅输出
  dedic_gpio_bundle_config_t bundleA_config = {
      .gpio_array = bundleA_gpios,
      .array_size = sizeof(bundleA_gpios) / sizeof(bundleA_gpios[0]),
      .flags = {
          .out_en = 1,
      },
  };
  ESP_ERROR_CHECK(dedic_gpio_new_bundle(&bundleA_config, &bundleA));
}

// static inline __attribute__((always_inline)) void soft_fast_gpio_write(uint8_t _8bit)
// {
//   // extern dedic_gpio_bundle_handle_t bundleA;
//   dedic_gpio_bundle_write(bundleA, 0b00111111, _8bit);
// }

#define soft_fast_gpio_write(_8bit) dedic_gpio_bundle_write(bundleA, 0b00111111, _8bit)

auto cost1 = micros();
auto cost2 = micros();
auto cost0 = portGET_RUN_TIME_COUNTER_VALUE();
auto cost3 = portGET_RUN_TIME_COUNTER_VALUE();
auto cost4 = portGET_RUN_TIME_COUNTER_VALUE();
auto cost5 = portGET_RUN_TIME_COUNTER_VALUE();

// Function to flush the framebuffer to the display
// cost 300+ms
void flushDisplay()
{
  auto counter = portGET_RUN_TIME_COUNTER_VALUE();

  uint8_t *buffer = canvas->getBuffer();
  digitalWrite(VST_PIN, HIGH);
  // delayMicroseconds(20000);
  vckState = !vckState;
  digitalWrite(VCK_PIN, vckState);

  for (int i = 1; i <= 487; i++)
  {
    if (i == 1)
    {
      // digitalWrite(VST_PIN, LOW);
      directWriteLow(VST_PIN);
      // delayMicroseconds(21000);
    }

    if (i >= 2 && i <= 481)
    {
      // digitalWrite(HST_PIN, HIGH);
      directWriteHigh(HST_PIN);
      hckState = !hckState; // Toggle HCK state
      // digitalWrite(HCK_PIN, hckState);
      if (hckState)
      {
        directWriteHigh(HCK_PIN);
      }
      else
      {
        directWriteLow(HCK_PIN);
      }

      // delayMicroseconds(1000);
      // 121 -> 123 了
      // 夏普需要123，并且一定是123
      // JDI 是 121
      cost0 = portGET_RUN_TIME_COUNTER_VALUE();
      cost3 = portGET_RUN_TIME_COUNTER_VALUE() - cost0; // 19 -> 76ns
      for (int j = 1; j <= 123; j++)
      {
        if (j == 1)
        {
          // digitalWrite(HST_PIN, LOW);
          directWriteLow(HST_PIN);
        }
        // 夏普屏在最后 ENB，JDI 在 20个 CLOCK的时候做 ENB
        if (j == 122)
        {
          // digitalWrite(ENB_PIN, HIGH);
          directWriteHigh(ENB_PIN);
        }
        else if (j == 123)
        {
          // digitalWrite(ENB_PIN, LOW);
          directWriteLow(ENB_PIN);
        }

        if (j >= 2 && j <= 121)
        {
          int y = (i - 2) / 2;
          int x = j - 2;
          int pos = (x * 2) + 240 * y;

          // Send pixel data
          uint8_t pixel = buffer[pos];
          // 这里 JDI 和 夏普屏都需要有 LPB/SPB 的区别，所以必须有下面的 if/else
          uint8_t _6bitColor = 0;
          if (i % 2 == 0)
          {
            // LPB
            if (USE_S3_DEDICATED_GPIO)
            {
              // _6bitColor |= (pixel & 0b100000) ? 0b100000 : 0;
              // _6bitColor |= (pixel & 0b001000) ? 0b001000 : 0;
              // _6bitColor |= (pixel & 0b000010) ? 0b000010 : 0;
              // simplify to below
              _6bitColor |= (pixel & 0b101010);
              pixel = buffer[pos + 1];
              // _6bitColor |= (pixel & 0b100000) ? 0b010000 : 0;
              // _6bitColor |= (pixel & 0b001000) ? 0b000100 : 0;
              // _6bitColor |= (pixel & 0b000010) ? 0b000001 : 0;
              // simplify to below
              _6bitColor |= (pixel & 0b101010) >> 1;
              // cpu tick is 4ns
              // cost0 = portGET_RUN_TIME_COUNTER_VALUE();
              // cost3 = portGET_RUN_TIME_COUNTER_VALUE() - cost0; // 20 -> 80ns
              soft_fast_gpio_write(_6bitColor);
              // cost4 = portGET_RUN_TIME_COUNTER_VALUE() - cost0; // 64 -> 64*4 = 256ns
            }
            else
            {
              digitalWrite(R1_PIN, (pixel & 0b100000) ? HIGH : LOW);
              digitalWrite(G1_PIN, (pixel & 0b001000) ? HIGH : LOW);
              digitalWrite(B1_PIN, (pixel & 0b000010) ? HIGH : LOW);
              pixel = buffer[pos + 1];
              digitalWrite(R2_PIN, (pixel & 0b100000) ? HIGH : LOW);
              digitalWrite(G2_PIN, (pixel & 0b001000) ? HIGH : LOW);
              digitalWrite(B2_PIN, (pixel & 0b000010) ? HIGH : LOW);
            }
          }
          else
          {
            // SPB
            if (USE_S3_DEDICATED_GPIO)
            {
              // _6bitColor |= (pixel & 0b010000) ? 0b100000 : 0;
              // _6bitColor |= (pixel & 0b000100) ? 0b001000 : 0;
              // _6bitColor |= (pixel & 0b000001) ? 0b000010 : 0;
              // simplify to below
              _6bitColor |= (pixel & 0b010101) << 1;
              pixel = buffer[pos + 1];
              // _6bitColor |= (pixel & 0b010000) ? 0b010000 : 0;
              // _6bitColor |= (pixel & 0b000100) ? 0b000100 : 0;
              // _6bitColor |= (pixel & 0b000001) ? 0b000001 : 0;
              // simplify to below
              _6bitColor |= (pixel & 0b010101);
              soft_fast_gpio_write(_6bitColor);
            }
            else
            {
              digitalWrite(R1_PIN, (pixel & 0b010000) ? HIGH : LOW);
              digitalWrite(G1_PIN, (pixel & 0b000100) ? HIGH : LOW);
              digitalWrite(B1_PIN, (pixel & 0b000001) ? HIGH : LOW);
              pixel = buffer[pos + 1];
              digitalWrite(R2_PIN, (pixel & 0b010000) ? HIGH : LOW);
              digitalWrite(G2_PIN, (pixel & 0b000100) ? HIGH : LOW);
              digitalWrite(B2_PIN, (pixel & 0b000001) ? HIGH : LOW);
            }
          }
        }

        // Toggle HCK state
        // TODO
        // delayMicroseconds(500);
        hckState = !hckState;
        // digitalWrite(HCK_PIN, hckState);
        if (hckState)
        {
          directWriteHigh(HCK_PIN);
        }
        else
        {
          directWriteLow(HCK_PIN);
        }
      }
      cost4 = portGET_RUN_TIME_COUNTER_VALUE() - cost0; // 9753 ticks -> 9753*4 = 39012ns: 39us
    }
    else
    {
      // delayMicroseconds(1000);
    }

    // delayMicroseconds(500);
    // Toggle VCK state
    vckState = !vckState;
    // digitalWrite(VCK_PIN, vckState);
    if (vckState)
    {
      directWriteHigh(VCK_PIN);
    }
    else
    {
      directWriteLow(VCK_PIN);
    }
  }
  // 19800 us / 4709264 ticks * 1000 = 4.2 nano seconds per tick
  auto total_cost = portGET_RUN_TIME_COUNTER_VALUE() - counter; // 4709264 ticks, 19800 us
  Serial.printf("%ld %ld %ld %ld\n", cost0, cost3, cost4, total_cost);
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
void setup2()
{
  Serial.begin(115200);
  delay(2000);
}

void loop2()
{
  // gpio_deep_sleep_hold_dis()
  gpio_hold_en(GPIO_NUM_16);

  auto t = touchRead(1);
  Serial.printf("%d\n", t);
  delay(200);
}

void setup()
{
  Serial.begin(115200);
  // pinMode(BACKLIGHT_PIN, OUTPUT);
  ledcSetup(BACKLIGHT_LEDC_CHANNEL, BL_FREQ, resolution);
  ledcAttachPin(BACKLIGHT_PIN, BACKLIGHT_LEDC_CHANNEL);
  ledcWrite(BACKLIGHT_LEDC_CHANNEL, 1 << (resolution - 1) - 1);
  // digitalWrite(BACKLIGHT_PIN, HIGH);

  canvas = new GFXcanvas8(240, 240);
  // delay(500);
  Serial.println("after new GFXcanvas8 and delay");

  if (USE_CONST_VCOM_NOT_PWM)
  {
    initScreenPWMPins();
  }
  else
  {
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
      // initLedc();
    }
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

  if (USE_S3_DEDICATED_GPIO)
  {
    dedic_gpio_new();
  }
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
    flushDisplay();
    delay(3000);
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
  case 12:
    for (int repeat = 0; repeat < 1; repeat++)
    {
      for (int frame = 0; frame < 7; frame++)
      {
        memcpy(canvas->getBuffer(), anim_data_7[frame], 240 * 240);
        flushDisplay();
        delay(1000);
      }
    }
    // memcpy(canvas->getBuffer(), rainbow_image_data, 240 * 240);
    break;
  case 13:
    for (int repeat = 0; repeat < 10; repeat++)
    {
      for (int frame = 0; frame < 12; frame++)
      {
        memcpy(canvas->getBuffer(), anim_data[frame], 240 * 240);
        flushDisplay();
        delay(80);
      }
    }
    // memcpy(canvas->getBuffer(), rainbow_image_data, 240 * 240);
    break;
  case 14:
    testFillRainbow();
    flushDisplay();
    delay(3000);
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
  if(frame_index >= 15) {
    digitalWrite(XRST_PIN, LOW);
    gpio_hold_en((gpio_num_t)XRST_PIN);
    gpio_hold_en((gpio_num_t)VCOM_PIN);
    gpio_hold_en((gpio_num_t)XFRP_PIN);
    gpio_hold_en((gpio_num_t)FRP_PIN);
    // gpio_deep_sleep_hold_en();
    // esp_deep_sleep(3000000);
    esp_sleep_enable_timer_wakeup(SLEEP_DURATION * 1000000ULL);  
    Serial.println("Going to sleep now");
    esp_deep_sleep_start();
    frame_index = 0;
  }
  delay(500);
  Serial.printf("flushDisplay took %d us\n", micros() - t);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  Serial.println("after LED");
}
