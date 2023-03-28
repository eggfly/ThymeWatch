#pragma once


#include <Adafruit_GFX.h>
#include <Adafruit_SharpMem.h>


#define DISPLAY_TYPE_MONO

// any pins can be used
#define SHARP_SCK     6
#define SHARP_MOSI    7
#define SHARP_SS      8
#define SHARP_DISP    10


#if defined(DISPLAY_TYPE_MONO)
// for LS010B7DH04 or LS013B7DH03 or LS013B4DN04 (monochrome)
#define COLOR_DEPTH 1
#elif defined(DISPLAY_TYPE_8_COLORS)
// for LS013B7DH06 (8 colors)
#define COLOR_DEPTH 3
#else
#error "Please define DISPLAY_TYPE_MONO or DISPLAY_TYPE_8_COLORS"
#endif

// Set the size and color depth, e.g. 3 bits for LS013B7DH06 (8 colors 128x128 display)
Adafruit_SharpMem canvas(&SPI, SHARP_SS, 128, 128, COLOR_DEPTH, 2000000); // 2100000 ok, 2500000 wrong, 230000 LDO ok

GFXcanvas16 canvas1(32, 32);
