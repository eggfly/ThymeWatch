#ifndef _LS012B7DD06_LIB_H_
#define _LS012B7DD06_LIB_H_

// 
// A library for LS012BDD06 reflective LCD for ESP32 microcontrollers.
// Apparently the LCD tolerates 3.3V instead of the recommended 3.2V.
// 
// ESP-side hardware used by this library:
// 	- I2S:		1 device for RGB data, BCK and BSP
// 	- RMT:		3 channels for GCK and GSP and INTB
// 	- MCPWM:	2 timers, 2 operators, 2 comparators and
// 				2 generators for VA and (VB, VCOM) as one signal
// 
// LCD pinout:
// 	1.	VDD2	- 5.0V DC power supply for the vertical driver
// 	2.	-		- open (no connection)
// 	3.	GSP		- start signal for the gate driver
// 	4.	GCK		- gate driver clock
// 	5.	GEN		- gate enable
// 	6.	INTB	- init signal for binary/gate driver
// 	7.	VB		- 0-3.2V power input; black signal voltage
// 					(inphase to VCOM), 50% duty square wave
// 	8.	VA		- 0-3.2V power input; white signal voltage
// 					(opposite phase to VCOM), 50% duty square wave
// 	9.	VDD1	- 3.2V DC power supply for the horizontal driver and pixel memory
// 	10.	VSS		- GND
// 	11.	BSP		- start signal for the binary driver
// 	12. BCK		- binary driver clock
// 	13.	R[0]	- red signal for odd pixels
// 	14.	R[1]	- red signal for even pixels
// 	15.	G[0]	- green signal for odd pixels
// 	16.	G[1]	- green signal for even pixels
// 	17.	B[0]	- blue signal for odd pixels
// 	18.	B[1]	- blue signal for even pixels
// 	19.	-		- open (no connection)
// 	20.	VCOM	- 0-3.2V power input; common terminal voltage for the LCD,
// 					50% duty square wave
// 	21.	(NC)	- open (no connection)
// 

// 
// Known bugs / errors:
// 
// The LCD doesn't work sometimes.
// It's caused by the software not sending any (or sometimes only BCK) data over I2S.
// Probably due to system tick timing.
// Looks like uncommenting, flashing and commenting again tx/rx_msb_right and 
// tx/rx_chan_mod variables in i2s_parallel_config_t in i2s_parallel.h along with
// 'I2S init done with flags' line and its commented version in *_hal.c helped.
// 

#include "config.h"

#include "graphics.h"

#include "esp_log.h"

// #include "freertos/semphr.h"	// for mutex

// static SemaphoreHandle_t mutex_handle_lcd;

// extern void rlcd_init( void );
extern void rlcd_testGPIOs( uint8_t pins_state );

extern void rlcd_sendFrame( void );
// extern void testTransmit( void );

extern void rlcd_init( void );

extern void togglePWM( void );

extern void rlcd_fillImageWhite( void );
extern void rlcd_fillImageColour( uint8_t colour );
extern void rlcd_putPixel( int16_t x, int16_t y, uint8_t colour );
extern void rlcd_updateImageBuf( void );

extern void rlcd_resume( void );
extern void rlcd_suspend( void );

// int rlcd_vprintf_func( const char *szFormat, va_list args );

#endif // _LS012B7DD06_LIB_H_