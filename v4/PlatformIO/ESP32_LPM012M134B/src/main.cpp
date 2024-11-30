#include <Arduino.h>


#define USE_LEDC_PWM (1)
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
#define XFRP_PIN 19
#define LED_PIN 2

// Frame buffer for 240x240 RGB222 display
uint8_t fb[240 * 240]; // 1 byte per pixel (RGB)


void resetDisplay()
{
  digitalWrite(XRST_PIN, HIGH);
  delayMicroseconds(500);
  digitalWrite(XRST_PIN, LOW);
  delayMicroseconds(23000);
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

#define FREQ_VCOM 60

// put function definitions here:
void anotherTask(void *parameter)
{
  pinMode(VCOM_PIN, OUTPUT);
  pinMode(FRP_PIN, OUTPUT);
  pinMode(XFRP_PIN, OUTPUT);
  while (true)
  {
    delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    digitalWrite(VCOM_PIN, HIGH);
    digitalWrite(FRP_PIN, HIGH);
    digitalWrite(XFRP_PIN, LOW);
    delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    digitalWrite(VCOM_PIN, LOW);
    digitalWrite(FRP_PIN, LOW);
    digitalWrite(XFRP_PIN, HIGH);
  }
}

// Function to flush the framebuffer to the display
void flushDisplay()
{
  digitalWrite(VST_PIN, HIGH);
  delayMicroseconds(2);
  digitalWrite(VCK_PIN, LOW);

  // State variables for toggling
  bool hckState = LOW;
  bool vckState = LOW;

  for (int i = 1; i <= 487; i++)
  {
    if (i == 1)
    {
      digitalWrite(VST_PIN, LOW);
    }

    if (i >= 2 && i <= 481)
    {
      digitalWrite(HST_PIN, HIGH);
      hckState = !hckState; // Toggle HCK state
      digitalWrite(HCK_PIN, hckState);

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
          int pos = (x * 2) + 240 * y;

          // Send pixel data
          uint8_t pixel = fb[pos];
          if (i % 2 == 0)
          {
            // LPB
            digitalWrite(R1_PIN, (pixel & 0b010000) ? HIGH : LOW);
            digitalWrite(G1_PIN, (pixel & 0b000100) ? HIGH : LOW);
            digitalWrite(B1_PIN, (pixel & 0b000001) ? HIGH : LOW);
            pixel = fb[pos + 1];
            digitalWrite(R2_PIN, (pixel & 0b010000) ? HIGH : LOW);
            digitalWrite(G2_PIN, (pixel & 0b000100) ? HIGH : LOW);
            digitalWrite(B2_PIN, (pixel & 0b000001) ? HIGH : LOW);
          }
          else
          {
            // SPB
            digitalWrite(R1_PIN, (pixel & 0b100000) ? HIGH : LOW);
            digitalWrite(G1_PIN, (pixel & 0b001000) ? HIGH : LOW);
            digitalWrite(B1_PIN, (pixel & 0b000010) ? HIGH : LOW);
            pixel = fb[pos + 1];
            digitalWrite(R2_PIN, (pixel & 0b100000) ? HIGH : LOW);
            digitalWrite(G2_PIN, (pixel & 0b001000) ? HIGH : LOW);
            digitalWrite(B2_PIN, (pixel & 0b000010) ? HIGH : LOW);
          }
        }

        // Toggle HCK state
        hckState = !hckState;
        digitalWrite(HCK_PIN, hckState);
      }
    }
    else
    {
      // delayMicroseconds(1000);
    }

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
      fb[index] = colors[currColorIndex];
    }
  }
  currColorIndex++;
  currColorIndex = currColorIndex % maxColorIndex;
  // Send the frame buffer to the display
  // flushDisplay();
}

const int freq = 60;      // PWM 频率
const int ledChannel = 0; // LEDC 通道
const int resolution = 8; // 分辨率（8 位）

void setup()
{
  Serial.begin(115200);
  delay(2000);
  if (!USE_LEDC_PWM)
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
  if (USE_LEDC_PWM)
  {
    ledcSetup(ledChannel, freq, resolution);
    ledcSetup(1, freq, resolution);
    delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    ledcSetup(2, freq, resolution);
    // 将 LEDC 通道连接到 GPIO
    ledcAttachPin(VCOM_PIN, ledChannel);
    ledcAttachPin(FRP_PIN, 1);
    delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    ledcAttachPin(XFRP_PIN, 2);

    ledcWrite(ledChannel, 128); // 设置 PWM 占空比
    ledcWrite(1, 128);
    // 高低电平半个周期
    delayMicroseconds(1000 * 1000 / FREQ_VCOM / 2);
    ledcWrite(2, 128);
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
  initDisplay();
}

void loop()
{
  Serial.println("before drawInitialContent");
  drawInitialContent();
  Serial.println("after drawInitialContent");
  Serial.println("before flushDisplay");
  auto t = micros();
  flushDisplay();
  Serial.printf("flushDisplay took %d us\n", micros() - t);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_PIN, LOW);
  Serial.println("after LED");
}
