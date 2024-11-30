
#include <Arduino.h>

#define WIDTH 240
#define HEIGHT 240

// Pin definitions
#define RST_PIN 7
#define VST_PIN 4
#define VCK_PIN 5
#define HST_PIN 12
#define HCK_PIN 13
#define ENB_PIN 6
#define R1_PIN 14
#define R2_PIN 15
#define G1_PIN 16
#define G2_PIN 17
#define B1_PIN 18
#define B2_PIN 21

uint8_t fb[WIDTH * HEIGHT];

// 定义PWM引脚
#define PWM_PIN_1 8 // 第一个PWM引脚 // FRP & VCOM
#define PWM_PIN_2 9 // 第二个PWM引脚 // xFRP

// LEDC通道定义
#define LEDC_CHANNEL_1 0
#define LEDC_CHANNEL_2 1

// LEDC定时器设置
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_FREQUENCY 60  // 60Hz
#define LEDC_RESOLUTION 12 // 12位分辨率

void reset()
{
  digitalWrite(RST_PIN, HIGH);
  delayMicroseconds(500);
  digitalWrite(RST_PIN, LOW);
  delayMicroseconds(23000);
  digitalWrite(RST_PIN, HIGH);
  delayMicroseconds(500);
}

void setup()
{
  Serial.begin(115200);
  pinMode(RST_PIN, OUTPUT);
  pinMode(VST_PIN, OUTPUT);
  pinMode(VCK_PIN, OUTPUT);
  pinMode(HST_PIN, OUTPUT);
  pinMode(HCK_PIN, OUTPUT);
  pinMode(ENB_PIN, OUTPUT);
  pinMode(R1_PIN, OUTPUT);
  pinMode(R2_PIN, OUTPUT);
  pinMode(G1_PIN, OUTPUT);
  pinMode(G2_PIN, OUTPUT);
  pinMode(B1_PIN, OUTPUT);
  pinMode(B2_PIN, OUTPUT);

  reset();
  init();

  ledcSetup(LEDC_CHANNEL_1, LEDC_FREQUENCY, LEDC_RESOLUTION);
  ledcAttachPin(PWM_PIN_1, LEDC_CHANNEL_1);

  ledcSetup(LEDC_CHANNEL_2, LEDC_FREQUENCY, LEDC_RESOLUTION);
  // ledcAttachPin(PWM_PIN_2, LEDC_CHANNEL_2);

  // 设置50%占空比
  ledcWrite(LEDC_CHANNEL_1, 2048); // 50% of 4095 (12-bit resolution)
  delayMicroseconds(1.0 / 120 * 1000);
  // ledcWrite(LEDC_CHANNEL_2, 2048);
}

void init()
{
  digitalWrite(VCK_PIN, LOW);
  digitalWrite(VST_PIN, LOW);
  digitalWrite(HST_PIN, LOW);
  digitalWrite(HCK_PIN, LOW);
  digitalWrite(ENB_PIN, LOW);
}

void flush()
{
  digitalWrite(VST_PIN, HIGH);
  delayMicroseconds(20000);
  digitalWrite(VCK_PIN, LOW);

  for (int i = 1; i <= 487; i++)
  {
    if (i == 1)
    {
      digitalWrite(VST_PIN, LOW);
      delayMicroseconds(21000);
    }

    if (i >= 2 && i <= 481)
    {
      digitalWrite(HST_PIN, HIGH);
      digitalWrite(HCK_PIN, HIGH);
      delayMicroseconds(1000);

      for (int j = 1; j <= 121; j++)
      {
        if (j == 1)
        {
          digitalWrite(HST_PIN, LOW);
        }

        if (j == 2)
        {
          digitalWrite(ENB_PIN, HIGH);
        }
        else if (j == 20)
        {
          digitalWrite(ENB_PIN, LOW);
        }

        if (j >= 1 && j <= 120)
        {
          int y = (i - 2) / 2;
          int x = j - 1;
          int pos = (x * 2) + WIDTH * y;

          uint8_t pixel = fb[pos];
          if (i % 2 == 0)
          { // LPB
            digitalWrite(R1_PIN, (pixel & 0b100000) ? HIGH : LOW);
            digitalWrite(G1_PIN, (pixel & 0b001000) ? HIGH : LOW);
            digitalWrite(B1_PIN, (pixel & 0b000010) ? HIGH : LOW);
            pixel = fb[pos + 1];
            digitalWrite(R2_PIN, (pixel & 0b100000) ? HIGH : LOW);
            digitalWrite(G2_PIN, (pixel & 0b001000) ? HIGH : LOW);
            digitalWrite(B2_PIN, (pixel & 0b000010) ? HIGH : LOW);
          }
          else
          { // SPB
            digitalWrite(R1_PIN, (pixel & 0b010000) ? HIGH : LOW);
            digitalWrite(G1_PIN, (pixel & 0b000100) ? HIGH : LOW);
            digitalWrite(B1_PIN, (pixel & 0b000001) ? HIGH : LOW);
            pixel = fb[pos + 1];
            digitalWrite(R2_PIN, (pixel & 0b010000) ? HIGH : LOW);
            digitalWrite(G2_PIN, (pixel & 0b000100) ? HIGH : LOW);
            digitalWrite(B2_PIN, (pixel & 0b000001) ? HIGH : LOW);
          }
        }

        delayMicroseconds(1);
        digitalWrite(HCK_PIN, LOW);
        digitalWrite(HCK_PIN, HIGH);
      }
    }
    else
    {
      delayMicroseconds(1000);
    }

    digitalWrite(VCK_PIN, LOW);
    digitalWrite(VCK_PIN, HIGH);
  }
}

void loop()
{
  // Example: Fill the framebuffer with a color
  for (int i = 0; i < WIDTH * HEIGHT; i++)
  {
    fb[i] = micros(); // Fill with red color
  }
  flush();
  Serial.println("before delay");
  delay(1000);
  Serial.println("after delay");
}
