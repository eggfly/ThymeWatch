#ifndef _I2S_PARALLEL_DRIVER_H_
#define _I2S_PARALLEL_DRIVER_H_

// 
// A I2S library for LS012B7DD06 reflective LCD based on
// har-in-air's ESP32-LCD-I2S project:
// https://github.com/har-in-air/ESP32-LCD-I2S
// bitluni's I2S library:
// https://github.com/bitluni/ESP32Lib
// and kargeor's "ESP32_I2S1_overclock.ino" sketch:
// https://gist.github.com/kargeor/b4200fc859a8e6c2234701368c82acd2
// 
// Used for sending image buffer to the LCD.
// Uses a separate output data buffer containing:
//  - encoded control signals (BSP and GEN) and
//  - encoded pixel data
// which is send to the display directly over I2S.
// 

#include <stdint.h>
#include "soc/i2s_struct.h"

#include "config.h"

typedef enum {
    I2S_PARALLEL_BITS_8 	= 8,
    I2S_PARALLEL_BITS_16	= 16,
    I2S_PARALLEL_BITS_32	= 32,
} i2s_parallel_cfg_bits_t;

typedef struct {
    lcd_colour_t* memory;
    size_t size;
} i2s_parallel_buffer_desc_t;

typedef struct {
    int gpio_bus[24];
    // I2S clock output pin
    int gpio_clk;

    uint32_t clock_speed_hz;

    // When set, makes WS active low
    // (adds a leading and trailing edge of the clock line).
    uint8_t tx_right_first;
    uint8_t rx_right_first;

    // Doesn't do anything
    // uint8_t tx_msb_right;
    // Doesn't do anything
    // uint8_t rx_msb_right;

    // uint8_t tx_chan_mod;
    // uint8_t rx_chan_mod;
    // uint16_t tx_msb_right : 1;
    // uint16_t rx_msb_right : 1;
    // uint16_t tx_chan_mod : 3;
    // uint16_t rx_chan_mod : 3;

    i2s_parallel_cfg_bits_t bits;
    i2s_parallel_buffer_desc_t* buf;
} i2s_parallel_config_t;

void i2s_stop( i2s_dev_t* dev );

void i2s_parallel_setup(i2s_dev_t* dev, const i2s_parallel_config_t* cfg);

void outDataBuf_clearImage( void );
void outDataBuf_update( void );
// void i2s_updateOutputBuf( i2s_dev_t *dev, bool all_black );

void i2s_prepareTx( i2s_dev_t *dev );
void i2s_startTx( i2s_dev_t *dev );
// void i2s_send_buf( i2s_dev_t* dev );

// void i2s_send_buf( i2s_dev_t *dev, i2s_parallel_buffer_desc_t* data_buf );
// void i2s_setStopSignal( bool state );
void i2s_setStopSignal( void );
bool getInvalidFlag( i2s_dev_t* dev, int* current_dma_desc );

#endif // _I2S_PARALLEL_DRIVER_H_