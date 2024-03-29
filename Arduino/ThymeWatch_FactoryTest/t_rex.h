#pragma once

#include "t_rex_gameover.h"
#include "t_rex_no_internet.h"

#define TFT_WHITE 0b111
#define TFT_BLACK 0b0

#define BTN_UP 4
#define BTN_DOWN 9

PROGMEM unsigned char dino[][1155] = {{
    0x00, 0x00, 0xF8, 0x3F, 0x00, 0x00, 0x00, 0xFC, 0x7F, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0x01, 0x00, 0x00, 0xEE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF,
    0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00,
    0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE,
    0xFF, 0x00, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00,
    0x01, 0x00, 0xFF, 0x1F, 0x00, 0x03, 0x80, 0x7F, 0x00, 0x00, 0x07, 0xE0,
    0x7F, 0x00, 0x00, 0x07, 0xF8, 0xFF, 0x00, 0x00, 0x0F, 0xFC, 0xFF, 0x03,
    0x00, 0x1F, 0xFE, 0xFF, 0x07, 0x00, 0x3F, 0xFF, 0x7F, 0x0E, 0x00, 0xFE,
    0xFF, 0x7F, 0x0C, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0xFF, 0x7F,
    0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0x00, 0xF8, 0xFF, 0x3F, 0x00, 0x00,
    0xF0, 0xFF, 0x3F, 0x00, 0x00, 0xE0, 0xFF, 0x0F, 0x00, 0x00, 0xE0, 0xFF,
    0x0F, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x80, 0xFF, 0x03, 0x00,
    0x00, 0x00, 0xBE, 0x03, 0x00, 0x00, 0x00, 0x0E, 0x03, 0x00, 0x00, 0x00,
    0x0E, 0x03, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x00, 0x06, 0x07,
    0x00, 0x00, 0x00, 0x1E, 0x0F, 0x00, 0x00,
  }, {
    0x00, 0x00, 0xF8, 0x3F, 0x00, 0x00, 0x00, 0xFC, 0x7F, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0x01, 0x00, 0x00, 0xEE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF,
    0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00,
    0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE,
    0xFF, 0x00, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00,
    0x01, 0x00, 0xFF, 0x1F, 0x00, 0x03, 0x80, 0x7F, 0x00, 0x00, 0x07, 0xE0,
    0x7F, 0x00, 0x00, 0x07, 0xF8, 0xFF, 0x00, 0x00, 0x0F, 0xFC, 0xFF, 0x03,
    0x00, 0x1F, 0xFE, 0xFF, 0x07, 0x00, 0x3F, 0xFF, 0x7F, 0x0E, 0x00, 0xFE,
    0xFF, 0x7F, 0x0C, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0xFF, 0x7F,
    0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0x00, 0xF8, 0xFF, 0x3F, 0x00, 0x00,
    0xF0, 0xFF, 0x3F, 0x00, 0x00, 0xE0, 0xFF, 0x0F, 0x00, 0x00, 0xE0, 0xFF,
    0x0F, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x80, 0xFF, 0x03, 0x00,
    0x00, 0x00, 0xBE, 0x03, 0x00, 0x00, 0x00, 0x0E, 0x0F, 0x00, 0x00, 0x00,
    0x0E, 0x0F, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00,
    0x00, 0x00, 0x00, 0x1E, 0x00, 0x00, 0x00,
  }, {
    0x00, 0x00, 0xF8, 0x3F, 0x00, 0x00, 0x00, 0xFC, 0x7F, 0x00, 0x00, 0x00,
    0xFE, 0xFF, 0x01, 0x00, 0x00, 0xEE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF,
    0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00,
    0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE, 0xFF, 0x01, 0x00, 0x00, 0xFE,
    0xFF, 0x00, 0x00, 0x00, 0xFE, 0x01, 0x00, 0x00, 0x00, 0xFE, 0x1F, 0x00,
    0x01, 0x00, 0xFF, 0x1F, 0x00, 0x03, 0x80, 0x7F, 0x00, 0x00, 0x07, 0xE0,
    0x7F, 0x00, 0x00, 0x07, 0xF8, 0xFF, 0x00, 0x00, 0x0F, 0xFC, 0xFF, 0x03,
    0x00, 0x1F, 0xFE, 0xFF, 0x07, 0x00, 0x3F, 0xFF, 0x7F, 0x0E, 0x00, 0xFE,
    0xFF, 0x7F, 0x0C, 0x00, 0xFE, 0xFF, 0x7F, 0x00, 0x00, 0xFE, 0xFF, 0x7F,
    0x00, 0x00, 0xFC, 0xFF, 0x7F, 0x00, 0x00, 0xF8, 0xFF, 0x3F, 0x00, 0x00,
    0xF0, 0xFF, 0x3F, 0x00, 0x00, 0xE0, 0xFF, 0x0F, 0x00, 0x00, 0xE0, 0xFF,
    0x0F, 0x00, 0x00, 0x80, 0xFF, 0x07, 0x00, 0x00, 0x80, 0xFF, 0x03, 0x00,
    0x00, 0x00, 0xBE, 0x03, 0x00, 0x00, 0x00, 0x0C, 0x03, 0x00, 0x00, 0x00,
    0x3C, 0x03, 0x00, 0x00, 0x00, 0x3C, 0x03, 0x00, 0x00, 0x00, 0x00, 0x07,
    0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00,
  }
};

PROGMEM unsigned char cloud[] = {
  0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x80, 0xCF, 0x01, 0x00, 0x00, 0xE0,
  0x01, 0x03, 0x00, 0x00, 0x60, 0x00, 0x0F, 0x00, 0x00, 0x38, 0x00, 0xFE,
  0x00, 0x00, 0x3C, 0x30, 0xA8, 0x03, 0xE0, 0x0F, 0x00, 0x80, 0x07, 0x70,
  0x05, 0x03, 0x00, 0x0E, 0x18, 0x00, 0x00, 0x02, 0x08, 0x0E, 0x00, 0x00,
  0x00, 0x18, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F,
};

PROGMEM unsigned char bump[][170] = {{
    0xC0, 0x01, 0x00, 0x1E, 0x00, 0x30, 0x03, 0x00, 0x33, 0x00, 0x1C, 0x0E,
    0xC0, 0xE0, 0x00, 0x06, 0x18, 0xF0, 0x80, 0x01, 0x03, 0xF0, 0x1F, 0x00,
    0x03,
  }, {
    0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x30, 0x0E,
    0xC0, 0x00, 0x00, 0x18, 0x98, 0xF3, 0xC1, 0x01, 0x07, 0xF0, 0x1F, 0x3E,
    0x03,
  }
};

PROGMEM unsigned char enemy[][684] =  {{
    0x00, 0x03, 0x00, 0x80, 0x07, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
    0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
    0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x8F, 0x01, 0xC6, 0xCF, 0x03,
    0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03,
    0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03,
    0xCF, 0xCF, 0x03, 0xCF, 0xCF, 0x03, 0xFF, 0xFF, 0x03, 0xFF, 0xFF, 0x01,
    0xFE, 0xFF, 0x00, 0xFC, 0x7F, 0x00, 0xF8, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
    0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
    0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
    0xC0, 0x0F, 0x00, 0xC0, 0x0F, 0x00,
  }, {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x03, 0x00, 0xC0, 0x07, 0x00,
    0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00, 0xC0, 0x47, 0x00, 0xC0, 0xE7, 0x00,
    0xC0, 0xE7, 0x00, 0xC0, 0xE7, 0x00, 0xC4, 0xE7, 0x00, 0xCE, 0xE7, 0x00,
    0xCE, 0xE7, 0x00, 0xCE, 0xE7, 0x00, 0xCE, 0xE7, 0x00, 0xCE, 0xE7, 0x00,
    0xCE, 0xF7, 0x00, 0xCE, 0xFF, 0x00, 0xCE, 0x7F, 0x00, 0xCE, 0x3F, 0x00,
    0xDE, 0x07, 0x00, 0xFE, 0x07, 0x00, 0xFC, 0x07, 0x00, 0xF8, 0x07, 0x00,
    0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00,
    0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00,
    0xC0, 0x07, 0x00, 0xC0, 0x07, 0x00,
  }
};

const int pwmFreq = 5000;
const int pwmResolution = 8;
const int pwmLedChannelTFT = 0;

int dinoW = 33;
int dinoH = 35;
float linesX[6];
int linesW[6];
float linesX2[6];
int linesW2[6];
float clouds[2] = {random(0, 80), random(100, 180)};
float bumps[2];
int bumpsF[2];
int eW = 18;
int eH = 38;


GFXcanvas16 img(240, 100);
GFXcanvas16 img2(33, 35);
GFXcanvas16 e(eW, eH);
GFXcanvas16 e2(eW, eH);


float eX[2] = {random(240, 310), random(380, 460)};
int ef[2] = {0, 1};

float sped = 1;
int x = 30;
int y = 58;
float dir = -1.4;
bool pressed = 0;
int frames = 0;
int f = 0;
float cloudSpeed = 0.4;
bool gameRun = 1;
static int t_rex_score = 0;
int t = 0;
int press2 = 0;

int brightnes[6] = {70, 100, 130, 160, 200, 220};
byte b = 1;


void t_rex_setup() {
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  //  tft.init();
  //  tft.setSwapBytes(true);
  canvas.fillScreen(TFT_WHITE);
  canvas.setRotation(0);
  //  img.setTextColor(TFT_BLACK, TFT_WHITE);
  //  img.setColorDepth(1);
  //  img2.setColorDepth(1);
  //  e.setColorDepth(1);
  //  e2.setColorDepth(1);



  // img.createSprite(240, 100);
  // img2.createSprite(33, 35);
  // e.createSprite(eW, eH);
  // e2.createSprite(eW, eH);
  canvas.fillScreen(TFT_WHITE);

  //  ledcSetup(pwmLedChannelTFT, pwmFreq, pwmResolution);
  //  ledcAttachPin(TFT_BL, pwmLedChannelTFT);
  //  ledcWrite(pwmLedChannelTFT, brightnes[b]);

  for (int i = 0; i < 6; i++) {
    linesX[i] = random(i * 40, (i + 1) * 40);
    linesW[i] = random(1, 14);
    linesX2[i] = random(i * 40, (i + 1) * 40);
    linesW2[i] = random(1, 14);
  }

  for (int n = 0; n < 2; n++) {
    bumps[n] = random(n * 90, (n + 1) * 120);
    bumpsF[n] = random(0, 2);
  }
  // tft.pushImage(0, 0, 217, 135, noInternet);
  canvas.drawRGBBitmap(0, 0, noInternet, 217, 135);
  canvas.refresh();
  //  while (digitalRead(BTN_UP) != 0) {
  //    if (digitalRead(BTN_DOWN) == 0) {
  //      if (press2 == 0) {
  //        press2 = 1;
  //        b++;
  //        if (b >= 6) {
  //          b = 0;
  //        }
  //        // ledcWrite(pwmLedChannelTFT, brightnes[b]);
  //      }
  //      delay(200);
  //    } else {
  //      press2 = 0;
  //    }
  //    // Serial.println("oo");
  //  }
  //  canvas.fillScreen(TFT_WHITE);
  //  Serial.println("oo2");

}


void drawS(int x, int y, int frame);
void checkColision();

void t_rex_loop() {
  if (gameRun == 1) {
    if (digitalRead(BTN_UP) == 0 && pressed == 0) {
      pressed = 1;
      f = 0;
    }

    if (pressed == 1) {
      y = y + dir;
      if (y == 2) {
        dir = dir * -1.00;
      }
      if (y == 58) {
        pressed = 0;
        dir = dir * -1.00;
      }
    }

    if (frames < 8 && pressed == 0) {
      f = 1;
    }
    if (frames > 8 && pressed == 0) {
      f = 2;
    }

    drawS(x, y, f);
    frames++;
    if (frames == 16) {
      frames = 0;
    }

    checkColision();
  }

  if (digitalRead(BTN_DOWN) == 0) {
    if (press2 == 0) {
      press2 = 1;
      b++;
      if (b >= 6) {
        b = 0;
      }
      // ledcWrite(pwmLedChannelTFT, brightnes[b]);
    }
  } else  {
    press2 = 0;
  }
}

void drawS(int x, int y, int frame) {
  img.fillScreen(TFT_WHITE);
  img.drawLine(0, 84, 240, 84, TFT_BLACK);

  for (int i = 0; i < 6; i++) {
    img.drawLine(linesX[i], 87 , linesX[i] + linesW[i], 87, TFT_BLACK);
    linesX[i] = linesX[i] - sped;
    if (linesX[i] < -14) {
      linesX[i] = random(245, 280);
      linesW[i] = random(1, 14);
    }
    img.drawLine(linesX2[i], 98 , linesX2[i] + linesW2[i], 98, TFT_BLACK);
    linesX2[i] = linesX2[i] - sped;
    if (linesX2[i] < -14) {
      linesX2[i] = random(245, 280);
      linesW2[i] = random(1, 14);
    }
  }
  for (int j = 0; j < 2; j++) {
    img.drawBitmap(clouds[j], 20, cloud, 38, 11, TFT_BLACK);
    clouds[j] = clouds[j] - cloudSpeed;
    if (clouds[j] < -40)
      clouds[j] = random(244, 364);
  }

  for (int n = 0; n < 2; n++) {
    img.drawBitmap(bumps[n], 80, bump[bumpsF[n]], 34, 5, TFT_BLACK);
    bumps[n] = bumps[n] - sped;
    if (bumps[n] < -40) {
      bumps[n] = random(244, 364);
      bumpsF[n] = random(0, 2);
    }
  }

  for (int m = 0; m < 2; m++) {
    eX[m] = eX[m] - sped;
    if (eX[m] < -20)
      eX[m] = random(240, 300);
    ef[m] = random(0, 2);
  }

  e.drawBitmap(0, 0, enemy[0], eW, eH, TFT_BLACK);
  e2.drawBitmap(0, 0, enemy[1], eW, eH, TFT_BLACK);
  img2.drawBitmap(0, 0, dino[frame], 33, 35, TFT_BLACK);


  canvas.drawRGBBitmap(eX[0], 56, e.getBuffer(), e.width(), e.height());
  // e.pushToSprite(&img, eX[0], 56, TFT_WHITE);
  canvas.drawRGBBitmap(eX[1], 56, e2.getBuffer(), e2.width(), e2.height());
  // e2.pushToSprite(&img, eX[1], 56, TFT_WHITE);
  canvas.drawRGBBitmap(x, y, img2.getBuffer(), img2.width(), img2.height());
  // img2.pushToSprite(&img, x, y, TFT_WHITE);

  t_rex_score = millis() / 120;
  canvas.setCursor(100, 0);
  canvas.println(String(t_rex_score));
  //img.drawString(String(sped),160,0,2);
  canvas.drawRGBBitmap(0, 17, img.getBuffer(), img.width(), img.height());
  // img.pushSprite(0, 17);

  if (t_rex_score > t + 100) {
    t = t_rex_score;
    sped = sped + 0.1;
  }

  // eggfly
  canvas.refresh();
}

void checkColision() {
  for (int i = 0; i < 2; i++) {
    if (eX[i] < x + dinoW / 2 && eX[i] > x && y > 25) {
      gameRun = 0;
      canvas.fillRect(0, 30, 240, 110, TFT_WHITE);
      canvas.drawBitmap(10, 30, gameover, 223, 100, TFT_BLACK);
      canvas.refresh();
      delay(500);
    }
  }
}
