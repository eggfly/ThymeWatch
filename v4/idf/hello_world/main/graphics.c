#include "graphics.h"

#include "font_5x7.h"

// Swap two uint16 macro:
#define swap( a, b ) { uint16_t t = a; a = b; b = t; }

// 
// Draw a line using Bresenham's algorithm.
// Coordinates as signed ints to make drawing large objects easier.
// x0, y0 - one end of the line
// x0, y0 - the other end of the line
// colour - colour of the line
// 
void rlcd_drawLine( int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint8_t colour ){

	int steep = abs( y1 - y0 ) > abs( x1 - x0 );

	if(steep){
		swap( x0, y0 );
		swap( x1, y1 );
	}

	if( x0 > x1 ){
		swap( x0, x1 );
		swap( y0, y1 );
	}

	int dx, dy;
	dx = x1 - x0;
	dy = abs( y1 - y0 );

	int err = dx / 2;
	int ystep;

	if( y0 < y1 )
		ystep = 1;
	else
		ystep = -1;

	for( ; x0 <= x1; x0++ ){
		if(steep)
			rlcd_putPixel( y0, x0, colour );
		else
			rlcd_putPixel( x0, y0, colour );

		err -= dy;

		if( err < 0 ){
			y0 += ystep;
			err += dx;
		}
	}
}

void rlcd_drawFillRect( int16_t x, int16_t y, int16_t w, int16_t h, uint8_t colour ){
	for( int16_t i = x; i < x + w; i++ ){
		rlcd_drawFastVLine( i, y, h, colour );
	}
}

void rlcd_drawFastVLine( int16_t x, int16_t y, uint8_t h, uint8_t colour ){
	for( int16_t i = y; i < y + h; i++ )
		rlcd_putPixel( x, i, colour );
	//rlcd_drawLine( x, y, x, y+h-1, colour );
}

void rlcd_drawFastHLine( int16_t x, int16_t y, uint8_t w, uint8_t colour ){
	for( int16_t i = x; i< x + w; i++ )
		rlcd_putPixel( x,i,colour );
	// rlcd_drawLine( x, y, x+w-1, y, colour );
}

void rlcd_drawChar( int16_t x, int16_t y, char c, uint8_t colour, uint8_t bg, uint8_t size ){

	if( (x >= RLCD_DISP_W) || (y >= RLCD_DISP_H) || ( (x + 6 * size - 1) < 0 ) || ( (y + 8 * size - 1) < 0 ) )
		return;

	uint8_t line;

	for( uint8_t i=0; i<6; i++ ){

		if( i == 5 )
			line = 0x0;
		else
			line = font_5x7[(c*5) + i]; //pgm_read_byte( font + (c*5) + i );

		for( int8_t j=0; j<8; j++ ){
			if( line & 0x1 ){
				if ( size == 1 )
					rlcd_putPixel( x+i, y+j, colour );
				else
					rlcd_drawFillRect( x+(i*size), y+(j*size), size, size, colour );
			}
			else if ( bg != colour ){
				if( size == 1 )
					rlcd_putPixel( x+i, y+j, bg );
				else
					rlcd_drawFillRect( x+i*size, y+j*size, size, size, bg );
			}
			line >>= 1;
		}
	}
}

void rlcd_putStr( int16_t x, int16_t y, char* str, uint8_t colour, uint8_t bg, uint8_t txt_size ){

	int16_t cursor_x = x, cursor_y = y;

	// Character counter to avoid writing
	// garbage data in an infinite loop.
	uint8_t char_cnt = 0;

	while( *str && char_cnt <= RLCD_STR_MAX_CHAR_CNT ){
		rlcd_drawChar( cursor_x, cursor_y, *str++, colour, bg, txt_size );
		cursor_x += txt_size*( FONT_WIDTH + FONT_SPACE_BETWEEN_CHARS );
		char_cnt++;
	}
}