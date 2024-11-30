#ifndef _LCD_GRAPHICS_H_
#define _LCD_GRAPHICS_H_

#include "ls012b7dd06_hal.h"

void rlcd_drawLine( int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t colour );
void rlcd_drawFastVLine( int16_t x, int16_t y, uint8_t h, uint8_t colour );
void rlcd_drawFastHLine( int16_t x, int16_t y, uint8_t w, uint8_t colour );

void rlcd_drawFillRect( int16_t x, int16_t y, int16_t w, int16_t h, uint8_t colour );

void rlcd_drawChar( int16_t x, int16_t y, char c, uint8_t colour, uint8_t bg, uint8_t size );
void rlcd_putStr( int16_t x, int16_t y, char* str, uint8_t colour, uint8_t bg, uint8_t txt_size );

#endif // _LCD_GRAPHICS_H_