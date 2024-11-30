#include "esp_log.h"            // for logging
#include "driver/gpio.h"
#include "soc/gpio_periph.h"    // for GPIO_PIN_MUX_REG[]
#include "rom/gpio.h"           // for gpio_matrix_out()

#include "esp_intr_alloc.h"     // for I2S interrupts

#include "rom/ets_sys.h"        // for us delay

#include "soc/i2s_struct.h"     // I2S API
#include "soc/i2s_reg.h"        // I2S definitions of registers
#include "driver/periph_ctrl.h"
#include "soc/io_mux_reg.h"
#include "rom/lldesc.h"         // for I2S DMA linked-list descriptors
#include "esp_heap_caps.h"      // for memory allocation

#include "driver/gpio.h"    // for testing purposes

#include "freertos/FreeRTOS.h"  // for vTaskDelay()
#include "freertos/task.h"      //

// For APLL clock
#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"

#include "soc/syscon_reg.h"
#include "soc/rtc_cntl_struct.h"
// 

#include "i2s_parallel.h"

// #include "../ls012b7dd06.h"

// #define OUT_DATA_BUF_LINE_W ( RLCD_DISP_W + RGB_CLK_LEADING_DUMMY_PERIOD_CNT + RGB_CLK_TRAILING_DUMMY_PERIOD_CNT )
#define OUT_DATA_BUF_LINE_W ( RLCD_DISP_W/2 + RGB_CLK_LEADING_DUMMY_PERIOD_CNT + RGB_CLK_TRAILING_DUMMY_PERIOD_CNT )
#define OUT_DATA_BUF_SIZE ( RLCD_DISP_H*2 * OUT_DATA_BUF_LINE_W )

#define TAG "i2s_parallel"

#define DMA_MAX (4096-4)

i2s_parallel_buffer_desc_t* image_buf_desc = NULL;

typedef struct {
    // Array of DMA buffer descriptors, not buffers!
    volatile lldesc_t* dma_desc_array;
    // Number of DMA descriptors in dma_desc_array.
    uint16_t dma_desc_count;
} i2s_parallel_state_t;

static i2s_parallel_state_t* i2s_state[2] = {NULL, NULL};

// Stores the index of currently active
// (that has just been sent) DMA descriptor.
uint16_t active_dma_desc_idx = 0;

// I2S interrupt handle
intr_handle_t i2s_intr_handle = NULL;

// Software stop signal for active I2S device.
bool i2s_stop_signal = false;

// Set in ISR if invalid DMA descriptor has been encountered.
bool invalid_dma_desc_flag = false;
// Index of last invalid DMA descriptor;
// valid only if invalid_dma_desc_flag is true.
uint16_t invalid_dma_desc_idx = 0;

// A separate buffer containing pixel and some control signals data.
uint8_t out_data_buf[OUT_DATA_BUF_SIZE] = { 0x00 };

// Array of dummy (virtual) pixels in each line of, to save memory,
// only one of the upper quarters of the round display,
// since the display is verticaly and horizontaly symmetric.
const uint8_t rlcd_dummy_px_cnt[ RLCD_DISP_H / 2 ] = {
	109, 101, 95, 91, 87, 84, 81, 78, 75, 73, 
	70, 68, 66, 64, 62, 61, 59, 57, 55, 54, 
	52, 51, 50, 48, 47, 46, 44, 43, 42, 41, 
	40, 38, 37, 36, 35, 34, 33, 32, 31, 31, 
	30, 29, 28, 27, 26, 25, 25, 24, 23, 22, 
	22, 21, 20, 20, 19, 18, 18, 17, 16, 16, 
	15, 15, 14, 14, 13, 13, 12, 12, 11, 11, 
	10, 10, 9, 9, 8, 8, 8, 7, 7, 7, 
	6, 6, 6, 5, 5, 5, 4, 4, 4, 3, 
	3, 3, 3, 2, 2, 2, 2, 2, 1, 1, 
	1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// 
// Get I2S device number ID.
// dev  - pointer to I2S device that is in use
// Return 0 if I2S0 is used, 1 otherwise (if I2S1 is used).
// 
static int i2s_getDevNum(i2s_dev_t *dev) {
    return (dev == &I2S0)? 0 : 1;
}

// 
// Set software stop signal for I2S device.
// The device will be stopped when transmission of
// current DMA descriptor is completed.
// 
void i2s_setStopSignal( void ){
    i2s_stop_signal = true;
    // while(i2s_stop_signal);
}

// 
// Reset I2S device's TX, RX, FIFO and AHBM.
// dev  - pointer to I2S device that is in use
// 
void i2s_reset( i2s_dev_t *dev ){
    const unsigned long lc_conf_reset_flags = I2S_IN_RST_M | I2S_OUT_RST_M | I2S_AHBM_RST_M | I2S_AHBM_FIFO_RST_M;
    dev->lc_conf.val |= lc_conf_reset_flags;
    dev->lc_conf.val &= ~lc_conf_reset_flags;

    const unsigned long conf_reset_flags = I2S_RX_RESET_M | I2S_RX_FIFO_RESET_M | I2S_TX_RESET_M | I2S_TX_FIFO_RESET_M;
    dev->conf.val |= conf_reset_flags;
    dev->conf.val &= ~conf_reset_flags;

    while( dev->state.rx_fifo_reset_back );
    // ESP_LOGI( TAG, "i2s_reset() done, dev->lc_conf.val = %ld, dev->conf.val = %ld", dev->lc_conf.val, dev->conf.val );
}

// 
// Stop I2S device.
// 
void i2s_stop( i2s_dev_t *dev ){
    ESP_ERROR_CHECK( esp_intr_disable( i2s_intr_handle ) );
    // ESP_LOGI( TAG, "Freeing I2S interrupt..." );
    ESP_ERROR_CHECK( esp_intr_free( i2s_intr_handle ) );
    // ESP_LOGI( TAG, "Freeing I2S interrupt done." );
    i2s_reset( dev );
    dev->conf.rx_start = 0;
    dev->conf.tx_start = 0;
}

// 
// Print I2S DMA descriptor's info and store its index,
// if corresponding hardware flag is set,
// dev              - I2S device handle
// inv_dma_desc_idx - if the descriptor is invalid, its index in the descriptor array
// Return true if invalid DMA descriptor has been encountered,
// false otherwise.
// 
bool getInvalidFlag( i2s_dev_t* dev, int* inv_dma_desc_idx ){
    if( invalid_dma_desc_flag ){
        *inv_dma_desc_idx = invalid_dma_desc_idx;

        i2s_parallel_state_t* st = i2s_state[i2s_getDevNum(dev)];

        ESP_LOGW( TAG, "getInvalidFlag(): Got invalid DMA descriptor no. %d at %p:", invalid_dma_desc_idx, &(st->dma_desc_array[invalid_dma_desc_idx]) );
        ESP_LOGW( TAG, "getInvalidFlag(): \tlength = %d",   st->dma_desc_array[invalid_dma_desc_idx].length );
        ESP_LOGW( TAG, "getInvalidFlag(): \tsize = %d",     st->dma_desc_array[invalid_dma_desc_idx].size );
        ESP_LOGW( TAG, "getInvalidFlag(): \toffset = %d",   st->dma_desc_array[invalid_dma_desc_idx].offset );
        ESP_LOGW( TAG, "getInvalidFlag(): \tsosf = %d",     st->dma_desc_array[invalid_dma_desc_idx].sosf );
        ESP_LOGW( TAG, "getInvalidFlag(): \teof = %d",      st->dma_desc_array[invalid_dma_desc_idx].eof );
        ESP_LOGW( TAG, "getInvalidFlag(): \towner = %d",    st->dma_desc_array[invalid_dma_desc_idx].owner );
        ESP_LOGW( TAG, "getInvalidFlag(): \tbuf = %p",      st->dma_desc_array[invalid_dma_desc_idx].buf );
        ESP_LOGW( TAG, "getInvalidFlag(): \tnext = %p",     st->dma_desc_array[invalid_dma_desc_idx].qe.stqe_next );
    }
    bool temp = invalid_dma_desc_flag;
    invalid_dma_desc_flag = false;

    return temp;
}

// 
// Proceed to next DMA descriptor and
// stop I2S if stop signal occured.
// dev  - handle to the I2S device that is in use
// 
void i2s_goToNextDMADesc( i2s_dev_t* dev ){

    /* // Go to next image line or sth

    // If all descriptors have been transmitted,
    // stop the I2S device.
    // if( i2s_state[ i2s_getDevNum( dev ) ]->dma_desc_array[active_dma_desc_idx].eof ){
    //     dev->conf.rx_start = 0;
    //     dev->conf.tx_start = 0;
        // i2s_reset( dev );
        // dev->conf.rx_reset = 1;
        // dev->conf.tx_reset = 1;
        // dev->conf.rx_reset = 0;
        // dev->conf.tx_reset = 0;
    //     dev->conf.rx_start = 0;
    //     dev->conf.tx_start = 0;
        //     i2s_stop( dev );
    // }
    */

    // Update active DMA descriptor index.
    active_dma_desc_idx = (active_dma_desc_idx + 1) % i2s_state[ i2s_getDevNum( dev ) ]->dma_desc_count;

    /* // Start transmission of another DMA descriptor
    if( active_dma_desc_idx != 0 ){
        // i2s_stop( dev );
        // ESP_ERROR_CHECK( esp_intr_disable( i2s_intr_handle ) );
        // i2s_reset( dev );
        dev->conf.rx_start = 0;
        dev->conf.tx_start = 0;

        // Disable the device to take control over GPIO pin for a moment

        // if ( dev == &I2S0 )
        //     periph_module_disable(PERIPH_I2S0_MODULE);
	    // else
        //     periph_module_disable(PERIPH_I2S1_MODULE);

        gpio_set_level( GPIO_NUM_20, 1 );   // GEN
        
        ets_delay_us(8);

        gpio_set_level( GPIO_NUM_20, 0 );

        // if ( dev == &I2S0 )
        //     periph_module_enable(PERIPH_I2S0_MODULE);
	    // else
        //     periph_module_enable(PERIPH_I2S1_MODULE);

        // This should take around 4us
        // i2s_send_buf( dev );

        // Set the DMA active buffer/descriptor to the next one 
        dev->out_link.addr = ((uint32_t)(&i2s_state[i2s_getDevNum( dev )]->dma_desc_array[active_dma_desc_idx]));
        dev->out_link.start = 1;                // and start DMA link

        // ESP_ERROR_CHECK( esp_intr_enable( i2s_intr_handle ) );

        // Start transmission
        dev->conf.tx_start = 1;
    }
    */

    // If i2s_setStopSignal() has been manually called,
    // stop the I2S device.
    if( i2s_stop_signal ){
        i2s_stop( dev );

        i2s_stop_signal = false;
    }
}

// 
// I2S ISR (interrupt handler function).
// Update invalid DMA descriptor flag,
// clear I2S interrupt status register and
// keep track of currently active DMA descriptor.
// arg  - pointer to currently active I2S device
// 
void IRAM_ATTR i2s_isr( void *arg ){
    if( ( (i2s_dev_t*)arg )->int_raw.out_dscr_err ){
        invalid_dma_desc_flag = true;
        invalid_dma_desc_idx = active_dma_desc_idx;
    }

    ( (i2s_dev_t*)arg )->int_clr.val = ( (i2s_dev_t*)arg )->int_raw.val;

    /* i2s_dev_t* dev = (i2s_dev_t*)arg;

    // Update active DMA descriptor index.
    active_dma_desc_idx = (active_dma_desc_idx + 1) % i2s_state[ i2s_getDevNum( dev ) ]->dma_desc_count;

    
    // Start transmission of another DMA descriptor
    if( active_dma_desc_idx != 0 ){
        // i2s_stop( dev );
        // ESP_ERROR_CHECK( esp_intr_disable( i2s_intr_handle ) );
        // i2s_reset( dev );
        dev->conf.rx_start = 0;
        dev->conf.tx_start = 0;

        // Disable the device to take control over GPIO pin for a moment

        // if ( dev == &I2S0 )
        //     periph_module_disable(PERIPH_I2S0_MODULE);
	    // else
        //     periph_module_disable(PERIPH_I2S1_MODULE);

        gpio_set_level( GPIO_NUM_20, 1 );   // GEN
        
        ets_delay_us(8);

        gpio_set_level( GPIO_NUM_20, 0 );

        // if ( dev == &I2S0 )
        //     periph_module_enable(PERIPH_I2S0_MODULE);
	    // else
        //     periph_module_enable(PERIPH_I2S1_MODULE);

        // This should take around 4us
        // i2s_send_buf( dev );

        // Set the DMA active buffer/descriptor to the next one 
        dev->out_link.addr = ((uint32_t)(&i2s_state[i2s_getDevNum( dev )]->dma_desc_array[active_dma_desc_idx]));
        dev->out_link.start = 1;                // and start DMA link

        // ESP_ERROR_CHECK( esp_intr_enable( i2s_intr_handle ) );

        // Start transmission
        dev->conf.tx_start = 1;
    }
    */

    i2s_goToNextDMADesc( (i2s_dev_t*)arg );
}

/* Unused functions
// 
// Calculate the number of DMA descriptors needed for a buffer descriptor.
// 
static int calc_needed_dma_descs_count( i2s_parallel_buffer_desc_t* desc ) {
	return ( desc->size + DMA_MAX - 1 ) / DMA_MAX;
}

// 
// Initialize a single empty DMA descriptor.
// Later, it will store data of MSB or LSB of a single image line.
// 
static void init_empty_dma_desc( volatile lldesc_t dma_desc ) {
	// int n = 0;
	// int len = bufdesc->size;
	// uint8_t* data_ptr = (uint8_t*)bufdesc->memory;

	// while(len) {
		// int dmalen = len;
		// if (dmalen > DMA_MAX) dmalen = DMA_MAX;
        dma_desc.length = OUT_DATA_BUF_LINE_W;              //dmalen;
		dma_desc.size = dma_desc.length;            //dmalen;
		dma_desc.buf = NULL;    //(uint8_t*)bufdesc->memory;   //data_ptr;

        // if( n == 0 )
            dma_desc.sosf = 1;
        // else 
        //     dma_desc.sosf = 0;
        
        dma_desc.eof = 1;   //0;

		dma_desc.owner = 1;

        // if( desc_count > 1 && n < desc_count )
		    // dma_desc.qe.stqe_next = (lldesc_t*) &dma_desc_array[n+1];
        // else
            dma_desc.qe.stqe_next = NULL;
        
		dma_desc.offset = 0;
		// len -= dmalen;
		// data_ptr += dmalen;
		// n++;
	// }

    // dma_desc_array[n-1].eof = 1;

    // Loop last back to first
    // dma_desc_array[n-1].qe.stqe_next = (lldesc_t*) &dma_desc_array[0];
    // ESP_LOGD( TAG, "init_dma_desc_array(): filled %d descriptor(s)", n );
}


// 
// Init DMA descriptor array.
// dma_desc_array   - array of DMA descriptors
// buffer_desc      - pointer to an image buffer descriptor
// desc_count       - size of this array
// 
static void init_dma_desc_array(volatile lldesc_t* dma_desc_array, i2s_parallel_buffer_desc_t* bufdesc, int desc_count ) {
	int n = 0;
	int len = bufdesc->size;
	uint8_t* data_ptr = (uint8_t*)bufdesc->memory;

	while(len) {
		int dmalen = len;
		if (dmalen > DMA_MAX) dmalen = DMA_MAX;
		dma_desc_array[n].size = dmalen;
		dma_desc_array[n].length = dmalen;
		dma_desc_array[n].buf = data_ptr;

        if( n == 0 )
            dma_desc_array[n].sosf = 1;
        else 
            dma_desc_array[n].sosf = 0;
        
        dma_desc_array[n].eof = 0;

		dma_desc_array[n].owner = 1;

        if( desc_count > 1 && n < desc_count )
		    dma_desc_array[n].qe.stqe_next = (lldesc_t*) &dma_desc_array[n+1];
        else
            dma_desc_array[n].qe.stqe_next = NULL;
        
		dma_desc_array[n].offset = 0;
		len -= dmalen;
		data_ptr += dmalen;
		n++;
	}

    dma_desc_array[n-1].eof = 1;

    // Loop last back to first
    // dma_desc_array[n-1].qe.stqe_next = (lldesc_t*) &dma_desc_array[0];
    ESP_LOGD( TAG, "init_dma_desc_array(): filled %d descriptor(s)", n );
}

*/

// 
// Init empty DMA descriptor array.
// Two descriptors per each image line (one for MSB and the other for LSB).
// dma_desc_array   - array of DMA descriptors
// desc_count       - size of this array
// 
static void dma_InitEmptyDescArray(volatile lldesc_t* dma_desc_array, uint16_t desc_count ) {
    for( uint16_t i = 0; i < desc_count; i++ ){

        dma_desc_array[i].length = OUT_DATA_BUF_LINE_W;
		dma_desc_array[i].size = dma_desc_array[i].length;
		dma_desc_array[i].buf = out_data_buf + ( i * OUT_DATA_BUF_LINE_W ); //NULL;

        if( i == 0 ){
            dma_desc_array[i].sosf = 1;
            dma_desc_array[i].qe.stqe_next = NULL;
        }
        else {
            dma_desc_array[i].sosf = 0;
            dma_desc_array[ i-1 ].qe.stqe_next = (lldesc_t*) &(dma_desc_array[i]);
        }

        // dma_desc_array[i].qe.stqe_next = NULL;

        // dma_desc_array[i].eof = 1;//0;
        dma_desc_array[i].eof = 0;
		dma_desc_array[i].owner = 1;
		dma_desc_array[i].offset = 0;
    }
    dma_desc_array[ desc_count - 1 ].eof = 1;
    dma_desc_array[ desc_count - 1 ].qe.stqe_next = NULL;
}

// 
// Allocate and fill calculated amount DMA descriptors with data from given buffer descriptor.
// dev  - I2S device handle
// buffer_desc - pointer to an image buffer descriptor
// 
// void dma_AllocateDescs( i2s_dev_t* dev, i2s_parallel_buffer_desc_t* buffer_desc ){
void dma_AllocateDescs( i2s_dev_t* dev ){
    i2s_parallel_state_t* st = i2s_state[i2s_getDevNum(dev)];
    // st->dma_desc_count = calc_needed_dma_descs_count(config->buf);
    // st->dma_desc_count = calc_needed_dma_descs_count(buffer_desc);
    st->dma_desc_count = RLCD_DISP_H * 2;

	// ESP_LOGD( TAG, "dma_AllocateDescs(): Number of descriptors = %d", st->dma_desc_count );
    ESP_LOGD( TAG, "dma_AllocateDescs(): Allocating %d descriptors...", st->dma_desc_count );

    st->dma_desc_array = heap_caps_malloc( st->dma_desc_count*sizeof(lldesc_t), MALLOC_CAP_DMA );
    // ... and fill them
    // init_dma_desc_array( st->dma_desc_array, config->buf, st->dma_desc_count ) ;

    // init_dma_desc_array( st->dma_desc_array, buffer_desc, st->dma_desc_count ) ;

    ESP_LOGD( TAG, "dma_AllocateDescs(): Initializing %d empty descriptors...", st->dma_desc_count );
    
    dma_InitEmptyDescArray( st->dma_desc_array, st->dma_desc_count );

    /*
    for( uint16_t i=0; i < st->dma_desc_count; i++ ){
        ESP_LOGD( TAG, "dma_AllocateDescs(): Descriptor no. %d at %p initialized with:", i, &(st->dma_desc_array[i]) );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \tlength = %d", st->dma_desc_array[i].length );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \tsize = %d", st->dma_desc_array[i].size );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \toffset = %d", st->dma_desc_array[i].offset );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \tsosf = %d", st->dma_desc_array[i].sosf );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \teof = %d", st->dma_desc_array[i].eof );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \towner = %d", st->dma_desc_array[i].owner );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \tbuf = %p", st->dma_desc_array[i].buf );
        ESP_LOGD( TAG, "dma_AllocateDescs(): \tnext = %p", st->dma_desc_array[i].qe.stqe_next );
    }
    */
    
    // st->dma_desc_array[ st->dma_desc_count - 1 ].qe.stqe_next = &(st->dma_desc_array[0]);

    ESP_LOGI( TAG, "dma_AllocateDescs(): Made a list of %d empty DMA descriptors.", st->dma_desc_count );
}

// 
// Swap bytes pairwise in the output I2S data buffer (out_data_buf[])
// because of transmission order from I2S's FIFO
// 
void outDataBuf_swapBytePairs( void ){
    // ESP_LOGD( TAG, "outDataBuf_swapBytePairs(): Swapping byte pairs..." );
    
    uint8_t temp1, temp2;
    for( uint32_t i = 0; i < OUT_DATA_BUF_SIZE; i += 4 ){
        temp1 = out_data_buf[i];
        temp2 = out_data_buf[i+1];

        out_data_buf[i] = out_data_buf[i+2];
        out_data_buf[i+1] = out_data_buf[i+3];

        out_data_buf[i+2] = temp1;
        out_data_buf[i+3] = temp2;
    }
}

// 
// Prepare the output I2S data buffer (out_data_buf[]):
//  - encode control signals (BSP and GEN)
//  - swap byte pairs
// Fill the array of DMA descriptors with image data.
// dev          - I2S device handle
// buffer_desc  - pointer to an image buffer descriptor
// 
void outDataBuf_prepare( void ){
    // i2s_parallel_state_t* st = i2s_state[i2s_getDevNum(dev)];

    ESP_LOGD( TAG, "outDataBuf_prepare(): Preparing output data buffer: size = %d, line width = %d...", OUT_DATA_BUF_SIZE, OUT_DATA_BUF_LINE_W );
    
    // vTaskDelay( 30 / portTICK_PERIOD_MS );

    ESP_LOGD( TAG, "outDataBuf_prepare(): Clearing the buffer..." );

    // Clear the output data buffer
    for( uint32_t i=0; i < OUT_DATA_BUF_SIZE; i++ )
        out_data_buf[i] = 0x00;

    ESP_LOGD( TAG, "outDataBuf_prepare(): Encoding control signals..." );

    // Encode control signals
    for( uint16_t line_no = 0; line_no < RLCD_DISP_H; line_no++ ){
        // Fill MSB:

        // Start of the whole MSB of image line data
        uint32_t out_buf_line_start_idx = line_no*2 * OUT_DATA_BUF_LINE_W;
        // End of MSB of image line data
        uint32_t out_buf_line_end_idx = out_buf_line_start_idx + OUT_DATA_BUF_LINE_W - 1;

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tFilling MSB of line %d: start_idx = %d,\tend_idx = %d,\trgb_start = %d,\trgb_end = %d...",
        //           line_no, out_buf_line_start_idx, out_buf_line_end_idx, out_buf_line_rgb_start_idx, out_buf_line_rgb_end_idx );

        // Encode BSP control signal
        for( uint8_t i=0; i < RGB_CLK_LEADING_DUMMY_PERIOD_CNT; i++ )
            out_data_buf[ out_buf_line_start_idx + i ] = ( 1 << GPIO_BUS_BSP_BIT );
        // Encode GEN control signal
        for( uint8_t i = 0; i < THHGCK_LEN; i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = 0x00;
        // During the last couple of periods GEN is low
        for( uint8_t i = THHGCK_LEN; i < ( THHGCK_LEN + THWGEN_LEN ); i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = ( 1 << GPIO_BUS_GEN_BIT );

        // Fill LSB:

        out_buf_line_start_idx += OUT_DATA_BUF_LINE_W;
        out_buf_line_end_idx = out_buf_line_start_idx + OUT_DATA_BUF_LINE_W - 1;

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tFilling LSB of line %d: start_idx = %d,\tend_idx = %d,\trgb_start = %d,\trgb_end = %d...",
        //           line_no, out_buf_line_start_idx, out_buf_line_end_idx, out_buf_line_rgb_start_idx, out_buf_line_rgb_end_idx );
        
        // Encode BSP control signal
        for( uint8_t i=0; i < RGB_CLK_LEADING_DUMMY_PERIOD_CNT; i++ )
            out_data_buf[ out_buf_line_start_idx + i ] = ( 1 << GPIO_BUS_BSP_BIT );
        // Encode GEN control signal
        for( uint8_t i = 0; i < THHGCK_LEN; i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = 0x00;
        // During the last couple of periods GEN is low
        for( uint8_t i = THHGCK_LEN; i < ( THHGCK_LEN + THWGEN_LEN ); i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = ( 1 << GPIO_BUS_GEN_BIT );
    }

    /* // All in one loop
    // For each image line
    for( uint16_t line_no = 0; line_no < RLCD_DISP_H; line_no++ ){
        // Fill with MSB:

        // Start of the whole MSB of image line data
        uint32_t out_buf_line_start_idx = line_no*2 * OUT_DATA_BUF_LINE_W;
        // End of MSB of image line data
        uint32_t out_buf_line_end_idx = out_buf_line_start_idx + OUT_DATA_BUF_LINE_W - 1;

        // Start of the pixel colour data in MSB of image line data
        uint32_t out_buf_line_rgb_start_idx = out_buf_line_start_idx + RGB_CLK_LEADING_DUMMY_PERIOD_CNT;
        // End of the pixel colour data in MSB of image line data
        uint32_t out_buf_line_rgb_end_idx = out_buf_line_end_idx - RGB_CLK_TRAILING_DUMMY_PERIOD_CNT;

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tFilling MSB of line %d: start_idx = %d,\tend_idx = %d,\trgb_start = %d,\trgb_end = %d...",
        //           line_no, out_buf_line_start_idx, out_buf_line_end_idx, out_buf_line_rgb_start_idx, out_buf_line_rgb_end_idx );

        // Encode BSP control signal
        for( uint8_t i=0; i < RGB_CLK_LEADING_DUMMY_PERIOD_CNT; i++ )
            out_data_buf[ out_buf_line_start_idx + i ] = ( 1 << GPIO_BUS_BSP_BIT );
        // Encode GEN control signal
        for( uint8_t i = 0; i < THHGCK_LEN; i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = 0x00;
        // During the last couple of periods GEN is low
        for( uint8_t i = THHGCK_LEN; i < ( THHGCK_LEN + THWGEN_LEN ); i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = ( 1 << GPIO_BUS_GEN_BIT );
        
        // Fill the buffer with MSB of pixel data in current line
        
        // out_data_buf[ out_buf_line_rgb_start_idx ] = buffer_desc->memory[ line_no ];

        if( !all_black ){
            for( uint32_t i = out_buf_line_rgb_start_idx; i < out_buf_line_rgb_end_idx; i+=4 ){
                out_data_buf[ i ] |= 0x01;
                out_data_buf[ i+1 ] |= 0x02;
                out_data_buf[ i+2 ] |= 0x04;
                out_data_buf[ i+3 ] |= 0x08;
            }
        }

        // Fill with LSB:

        out_buf_line_start_idx += OUT_DATA_BUF_LINE_W;
        out_buf_line_end_idx = out_buf_line_start_idx + OUT_DATA_BUF_LINE_W - 1;

        out_buf_line_rgb_start_idx = out_buf_line_start_idx + RGB_CLK_LEADING_DUMMY_PERIOD_CNT;
        out_buf_line_rgb_end_idx = out_buf_line_end_idx - RGB_CLK_TRAILING_DUMMY_PERIOD_CNT;

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tFilling LSB of line %d: start_idx = %d,\tend_idx = %d,\trgb_start = %d,\trgb_end = %d...",
        //           line_no, out_buf_line_start_idx, out_buf_line_end_idx, out_buf_line_rgb_start_idx, out_buf_line_rgb_end_idx );
        
        // Encode BSP control signal
        for( uint8_t i=0; i < RGB_CLK_LEADING_DUMMY_PERIOD_CNT; i++ )
            out_data_buf[ out_buf_line_start_idx + i ] = ( 1 << GPIO_BUS_BSP_BIT );
        // Encode GEN control signal
        for( uint8_t i = 0; i < THHGCK_LEN; i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = 0x00;
        // During the last couple of periods GEN is low
        for( uint8_t i = THHGCK_LEN; i < ( THHGCK_LEN + THWGEN_LEN ); i++ )
            out_data_buf[ out_buf_line_end_idx - i ] = ( 1 << GPIO_BUS_GEN_BIT );

        // Fill the buffer with LSB of pixel data in current line
        if( !all_black ){
            for( uint32_t i = out_buf_line_rgb_start_idx; i < out_buf_line_rgb_end_idx; i+=4 ){
                out_data_buf[ i ] |= 0x01;
                out_data_buf[ i+1 ] |= 0x02;
                out_data_buf[ i+2 ] |= 0x04;
                out_data_buf[ i+3 ] |= 0x08;
            }
        }
    }
    */
    
    // ESP_LOGD( TAG, "outDataBuf_prepare(): Swapping byte pairs..." );

    outDataBuf_swapBytePairs();

    /*
    ESP_LOGD( TAG, "outDataBuf_prepare(): Filling %d descriptors with image data...", st->dma_desc_count );
    // For each DMA descriptor (there are 2*OUT_DATA_BUF_LINE_W descriptors)
    for( uint16_t i = 0; i < st->dma_desc_count; i++ ){
        // For now:
        // Set the starting position in the image buffer.
        // st->dma_desc_array[i].buf = buffer_desc->memory + ( i * OUT_DATA_BUF_LINE_W );

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tDescriptor %d: start = out_data_buf + %d.", i, ( i * OUT_DATA_BUF_LINE_W ) );
        st->dma_desc_array[i].buf = out_data_buf + ( i * OUT_DATA_BUF_LINE_W );
    }
    */

    // ESP_LOGI( TAG, "outDataBuf_prepare(): Filled %d descriptors with image data.", st->dma_desc_count );
}

// 
// Clear image data in the output I2S data buffer.
// 
void outDataBuf_clearImage( void ){
    uint8_t rgb_pins_bit_mask = (1 << GPIO_BUS_R0_BIT) | (1 << GPIO_BUS_R1_BIT) |
                           (1 << GPIO_BUS_G0_BIT) | (1 << GPIO_BUS_G1_BIT) |
                           (1 << GPIO_BUS_B0_BIT) | (1 << GPIO_BUS_B1_BIT);
    
    
    for( uint32_t i=0; i < OUT_DATA_BUF_SIZE; i++ )
        out_data_buf[i] &= ~rgb_pins_bit_mask;
}

// 
// Fill the output I2S data buffer with image data.
// buffer_desc - pointer to buffer descriptor pointing to image buffer
// 
void IRAM_ATTR outDataBuf_encodeImage( i2s_parallel_buffer_desc_t* buffer_desc ){
    // Clear image data (temporary; until I manage to copy rlcd_buf into out_data_buf)

    // Temporarily until I manage to copy rlcd_buf into out_data_buf:
    // Unswap byte pairs for the time of encoding image
    outDataBuf_swapBytePairs();

    // uint32_t for_cnt = 0;

    // Encode image data
    for( uint16_t line_no = 0; line_no < RLCD_DISP_H; line_no++ ){
        // Fill with MSB:

        // Start of the whole MSB of image line data
        uint32_t out_buf_line_start_idx = line_no*2 * OUT_DATA_BUF_LINE_W;
        // End of MSB of image line data
        uint32_t out_buf_line_end_idx = out_buf_line_start_idx + OUT_DATA_BUF_LINE_W - 1;

        // Start of the pixel colour data in MSB of image line data
        uint32_t out_buf_line_rgb_start_idx = out_buf_line_start_idx + RGB_CLK_LEADING_DUMMY_PERIOD_CNT;
        // End of the pixel colour data in MSB of image line data
        uint32_t out_buf_line_rgb_end_idx = out_buf_line_end_idx - RGB_CLK_TRAILING_DUMMY_PERIOD_CNT;

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tFilling MSB of line %d: start_idx = %d,\tend_idx = %d,\trgb_start = %d,\trgb_end = %d...",
        //           line_no, out_buf_line_start_idx, out_buf_line_end_idx, out_buf_line_rgb_start_idx, out_buf_line_rgb_end_idx );

        // Fill the buffer with MSB of pixel data in current line
        
        // out_data_buf[ out_buf_line_rgb_start_idx ] = buffer_desc->memory[ line_no ];
        
        // for( uint32_t odb_i = out_buf_line_rgb_start_idx, imgb_i = line_no * RLCD_DISP_W;
        //      odb_i < out_buf_line_rgb_end_idx && imgb_i < RLCD_DISP_H * RLCD_DISP_W;
        //      odb_i++, imgb_i += 2 ){
        
        // Image buffer pixel iterator
        uint32_t imgb_i = line_no * RLCD_DISP_W;

        // Get the number of dummy pixels in current video line of the round display:
        uint8_t dummy_px_cnt;
        // Dummy pixel parity flag; set if the number of dummy pixels is even:
        uint8_t dummy_px_parity = 0;
        // If current line is in the upper half of the display:
        if( line_no < RLCD_DISP_H / 2 )
            dummy_px_cnt = rlcd_dummy_px_cnt[ line_no ];
        // Or if the current line is in the lower half of the display:
        else
            dummy_px_cnt = rlcd_dummy_px_cnt[ (RLCD_DISP_H-1) - line_no ];
        
        if( dummy_px_cnt % 2 == 0 )
            dummy_px_parity = 1;

        // Don't fill these dummy pixels with image data to save time:
        // Need to adjust these numbers or resign from using them here
        // and modify the for loop below
        out_buf_line_rgb_start_idx  += dummy_px_cnt/2;
        if( dummy_px_cnt > 1 )
            out_buf_line_rgb_end_idx    -= dummy_px_cnt/2;

        imgb_i += dummy_px_cnt;

        if( dummy_px_cnt > 0 && dummy_px_parity == 0 ){
            // out_buf_line_rgb_end_idx++;  // can't do this! is affects control signal data!
            imgb_i--;
        }


        // for_cnt = 0;

        // For each pixel's MSB in the output data buffer
        for( uint32_t odb_i = out_buf_line_rgb_start_idx; odb_i < out_buf_line_rgb_end_idx; odb_i++ ){

            // if( imgb_i >= RLCD_DISP_H * RLCD_DISP_W ){
            //     ESP_LOGW( TAG, "imgb_i = %lu overflow. Returning.", imgb_i );
            //     return;
            // }
            
            // if( odb_i = out_buf_line_rgb_start_idx && ( dummy_px_cnt % 2 ) )
            //     continue;

            // Odd pixels:
            
            // if( ( odb_i != out_buf_line_rgb_start_idx ) || (odb_i == out_buf_line_rgb_start_idx && dummy_px_parity ) ){
                if( buffer_desc->memory[imgb_i].bits.r_msb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_R0_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_R0_BIT);

                if( buffer_desc->memory[imgb_i].bits.g_msb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_G0_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_G0_BIT);
                
                if( buffer_desc->memory[imgb_i].bits.b_msb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_B0_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_B0_BIT);
            // }
            
            // Even pixels:

            // if( ( odb_i != (out_buf_line_rgb_end_idx - 1) ) || (odb_i == (out_buf_line_rgb_end_idx - 1) && dummy_px_parity ) ){
                if( buffer_desc->memory[imgb_i+1].bits.r_msb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_R1_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_R1_BIT);
                
                if( buffer_desc->memory[imgb_i+1].bits.g_msb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_G1_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_G1_BIT);
                
                if( buffer_desc->memory[imgb_i+1].bits.b_msb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_B1_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_B1_BIT);
            // }

            imgb_i += 2;

            // for_cnt++;
        }

        // Fill with LSB:

        out_buf_line_start_idx += OUT_DATA_BUF_LINE_W;
        out_buf_line_end_idx = out_buf_line_start_idx + OUT_DATA_BUF_LINE_W - 1;

        out_buf_line_rgb_start_idx = out_buf_line_start_idx + RGB_CLK_LEADING_DUMMY_PERIOD_CNT;
        out_buf_line_rgb_end_idx = out_buf_line_end_idx - RGB_CLK_TRAILING_DUMMY_PERIOD_CNT;

        // ESP_LOGD( TAG, "outDataBuf_prepare(): \tFilling LSB of line %d: start_idx = %d,\tend_idx = %d,\trgb_start = %d,\trgb_end = %d...",
        //           line_no, out_buf_line_start_idx, out_buf_line_end_idx, out_buf_line_rgb_start_idx, out_buf_line_rgb_end_idx );

        // Fill the buffer with LSB of pixel data in current line
        // for( uint32_t i = out_buf_line_rgb_start_idx; i < out_buf_line_rgb_end_idx; i+=4 ){
        /*
        for( uint32_t odb_i = out_buf_line_rgb_start_idx, imgb_i = line_no * RLCD_DISP_W;
             odb_i < out_buf_line_rgb_end_idx && imgb_i < RLCD_DISP_H * RLCD_DISP_W;
             odb_i++, imgb_i++ ){
            
            if( buffer_desc->memory[imgb_i].bits.r_lsb )
                out_data_buf[ odb_i ] |= (1 << COLOUR_R_LSB_BIT);
            else
                out_data_buf[ odb_i ] &= ~(1 << COLOUR_R_LSB_BIT);

            if( buffer_desc->memory[imgb_i].bits.g_lsb )
                out_data_buf[ odb_i ] |= (1 << COLOUR_G_LSB_BIT);
            else
                out_data_buf[ odb_i ] &= ~(1 << COLOUR_G_LSB_BIT);

            if( buffer_desc->memory[imgb_i].bits.b_lsb )
                out_data_buf[ odb_i ] |= (1 << COLOUR_B_LSB_BIT);
            else
                out_data_buf[ odb_i ] &= ~(1 << COLOUR_B_LSB_BIT);

            // out_data_buf[ i ] |= 0x01;
            // out_data_buf[ i+1 ] |= 0x02;
            // out_data_buf[ i+2 ] |= 0x04;
            // out_data_buf[ i+3 ] |= 0x08;
        }
        */
        imgb_i = line_no * RLCD_DISP_W;

        // Don't fill dummy pixels with image data to save time:
        out_buf_line_rgb_start_idx  += dummy_px_cnt/2;
        if( dummy_px_cnt > 1 )
            out_buf_line_rgb_end_idx    -= dummy_px_cnt/2;
        imgb_i += dummy_px_cnt;

        if( dummy_px_cnt > 0 && dummy_px_parity == 0 ){
            // out_buf_line_rgb_end_idx++;  // can't do this! is affects control signal data!
            imgb_i--;
        }
        
        for( uint32_t odb_i = out_buf_line_rgb_start_idx; odb_i < out_buf_line_rgb_end_idx; odb_i++ ){

            // if( imgb_i >= RLCD_DISP_H * RLCD_DISP_W ){
            //     ESP_LOGW( TAG, "imgb_i = %lu overflow. Returning.", imgb_i );
            //     return;
            // }

            // Odd pixels:

            // if( ( odb_i != out_buf_line_rgb_start_idx ) || (odb_i == out_buf_line_rgb_start_idx && dummy_px_parity ) ){
                if( buffer_desc->memory[imgb_i].bits.r_lsb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_R0_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_R0_BIT);

                if( buffer_desc->memory[imgb_i].bits.g_lsb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_G0_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_G0_BIT);
                
                if( buffer_desc->memory[imgb_i].bits.b_lsb )
                    out_data_buf[ odb_i ] |= (1 << GPIO_BUS_B0_BIT);
                else
                    out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_B0_BIT);
            // }

            // Even pixels:

            if( buffer_desc->memory[imgb_i+1].bits.r_lsb )
                out_data_buf[ odb_i ] |= (1 << GPIO_BUS_R1_BIT);
            else
                out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_R1_BIT);
            
            if( buffer_desc->memory[imgb_i+1].bits.g_lsb )
                out_data_buf[ odb_i ] |= (1 << GPIO_BUS_G1_BIT);
            else
                out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_G1_BIT);
            
            if( buffer_desc->memory[imgb_i+1].bits.b_lsb )
                out_data_buf[ odb_i ] |= (1 << GPIO_BUS_B1_BIT);
            else
                out_data_buf[ odb_i ] &= ~(1 << GPIO_BUS_B1_BIT);

            imgb_i += 2;
        }
    }

    // Temporarily until I manage to copy rlcd_buf into out_data_buf:
    // Swap byte pairs again 
    outDataBuf_swapBytePairs();

    // ESP_LOGI( TAG, "Iterated %lu for each line.", for_cnt );
}

// 
// Update the output data buffer with new image data.
// 
void outDataBuf_update( void ){//} i2s_dev_t *dev ){
    if( image_buf_desc == NULL ){
        ESP_LOGW( TAG, "outDataBuf_update(): image_buf_desc == NULL. Returning." );
        return;
    }

    outDataBuf_encodeImage( image_buf_desc );
    // outDataBuf_prepare( dev, image_buf_desc );
}

// 
// Set up GPIO pin as output.
// gpio - GPIO number
// sig  - signal
// inv  - pin logic inversion
// 
static void gpio_setup_out(int gpio, int sig, int inv) {
    if ( gpio == -1 )
        return;
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[gpio], PIN_FUNC_GPIO);
    gpio_set_direction(gpio, GPIO_MODE_DEF_OUTPUT);
    gpio_matrix_out(gpio, sig, inv, false);
}

// 
// Reset I2S device's DMA.
// dev  - pointer to I2S device that is in use
// 
static void dma_reset(i2s_dev_t *dev) {
    dev->lc_conf.in_rst=1; 
	dev->lc_conf.in_rst=0;
    dev->lc_conf.out_rst=1; 
	dev->lc_conf.out_rst=0;
}

// 
// Reset I2S device's FIFO.
// dev  - pointer to I2S device that is in use
// 
static void fifo_reset(i2s_dev_t *dev) {
    dev->conf.rx_fifo_reset=1; 
	dev->conf.rx_fifo_reset=0;
    dev->conf.tx_fifo_reset=1; 
	dev->conf.tx_fifo_reset=0;
}

// 
// Setup I2S device in parallel mode.
// dev  - pointer to I2S device that is in use
// config   - pointer to its configuration structure
// 
void i2s_parallel_setup( i2s_dev_t *dev, const i2s_parallel_config_t *config ) {
    // Figure out which signal numbers to use for routing
    ESP_LOGI( TAG, "i2s_parallel_setup(): Setting up parallel bus at I2S%d", i2s_getDevNum(dev) );

    int sig_data_base, sig_clk;

    if( dev == &I2S0 ) {
        sig_data_base = I2S0O_DATA_OUT0_IDX;
        sig_clk = I2S0O_WS_OUT_IDX;
	} 
	else {
        if (config->bits == I2S_PARALLEL_BITS_32 ) {
            sig_data_base = I2S1O_DATA_OUT0_IDX;
		} 
		else if (config->bits == I2S_PARALLEL_BITS_8 ) {
            sig_data_base = I2S1O_DATA_OUT0_IDX;
		} 
		else {
            // Because of... reasons... the 16-bit values for i2s1 appear on d8...d23
            sig_data_base = I2S1O_DATA_OUT8_IDX;
		}
        sig_clk = I2S1O_WS_OUT_IDX;
	}
    
    // Route the signals
    gpio_setup_out(config->gpio_bus[0], sig_data_base+0, false); // D0
    gpio_setup_out(config->gpio_bus[1], sig_data_base+1, false); // D1
    gpio_setup_out(config->gpio_bus[2], sig_data_base+2, false); // D2
    gpio_setup_out(config->gpio_bus[3], sig_data_base+3, false); // D3
    gpio_setup_out(config->gpio_bus[4], sig_data_base+4, false); // D4
    gpio_setup_out(config->gpio_bus[5], sig_data_base+5, false); // D5
    gpio_setup_out(config->gpio_bus[6], sig_data_base+6, false); // HS
    gpio_setup_out(config->gpio_bus[7], sig_data_base+7, false); // VS
		
    //ToDo: Clk/WS may need inversion?
    //gpio_setup_out(config->gpio_clk, sig_clk, true);
    gpio_setup_out(config->gpio_clk, sig_clk, false);
    
    //Power on dev
    if (dev == &I2S0) {
        periph_module_enable(PERIPH_I2S0_MODULE);
	} 
	else {
        periph_module_enable(PERIPH_I2S1_MODULE);
	}
    //Initialize I2S dev
    dev->conf.rx_reset = 1; 
	dev->conf.rx_reset = 0;
    dev->conf.tx_reset = 1; 
	dev->conf.tx_reset = 0;
    dma_reset(dev);
    fifo_reset(dev);
    
    dev->conf2.val = 0;
    dev->conf2.lcd_en = 1;          // Enable LCD mode
    dev->conf2.lcd_tx_wrx2_en = 1;  // '1' - makes the device transmit whole clock period per bit of data
                                    //     (that is slows down data lines 2x)
    dev->conf2.lcd_tx_sdx2_en = 0;  // '1' - makes the device transmit every bit of data twice
    
    dev->sample_rate_conf.val = 0;
    dev->sample_rate_conf.rx_bits_mod = config->bits;
    dev->sample_rate_conf.tx_bits_mod = config->bits;

    // 
    // Configure clock source, and clock divisors for I2S_CLK and I2SnO_BCK_out.
    // For details see section 12.3 of ESP32 technical reference manual.
    // 

    // Set I2SnO_BCK_out divisor value
    // 
    // f_I2SnO_BCK_out = 1 / M,
    // where M = dev->sample_rate_conf.tx_bck_div_num
    // 
    // The lowest value which gives stable results is 2.
    // Value 1 results in some phase-shift errors.
    dev->sample_rate_conf.rx_bck_div_num = 2; 
    dev->sample_rate_conf.tx_bck_div_num = 2;

    ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source to %ldHz:", config->clock_speed_hz );

    /* // APLL clock source stuff
    
    // ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Rebooting APLL clock..." );
    
    // Configure APLL clock
    // RTCCNTL.ana_conf.plla_force_pd = 0; // Power down APLL
    // RTCCNTL.ana_conf.plla_force_pu = 1; // Power up APLL
    
    // From esp-idf/components/esp_hw_support/port/esp32/rtc_clk.c/rtc_clk_apll_coeff_calc() we get:
    // apll_freq = xtal_freq * (4 + sdm2 + sdm1/256 + sdm0/65536) / ((o_div + 2) * 2)
    //             ----------------------------------------------   -----------------
    //                  350 MHz <= Numerator <= 500 MHz                Denominator
    // 
    // So f_APLL_CLK has to be greater than 5303031Hz.
    unsigned long expected_apll_freq = 10000000;

    uint32_t o_div;
    uint32_t sdm0;
    uint32_t sdm1;
    uint32_t sdm2;

    ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Calculating APLL coefficients for frequency %luHz...", expected_apll_freq );

    // Try to calculate APLL formula coefficients and, if succeeded, apply them.
    if( 1 == 0 && rtc_clk_apll_coeff_calc( expected_apll_freq, &o_div, &sdm0, &sdm1, &sdm2) ){
        ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: APLL coefficients found: o_div = %lu, sdm0 = %lu, sdm1 = %lu, sdm2 = %lu.",
                        o_div, sdm0, sdm1, sdm2 );

        // 40MHz is the default XTAL frequency
        float apll_freq = (float)40000000 * (4.0 + (float)sdm2 + (float)sdm1/256 + (float)sdm0/65536) / (float)((o_div + 2) * 2);
        ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Resulting APLL frequency: %fHz.", apll_freq );

        ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Applying APLL coefficients..." );
        rtc_clk_apll_coeff_set( o_div, sdm0, sdm1, sdm2);

        ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Enabling APLL..." );
        rtc_clk_apll_enable( true );

        dev->clkm_conf.val = 0;     // Reset I2S clock configuration
        dev->clkm_conf.clka_en = 1; // Switch to APLL clock source

        // f_I2Sn_CLK = f_APLL_CLK / (N + b/a)
        // where N = dev->clkm_conf.clkm_div_num
        dev->clkm_conf.clkm_div_a = 1;
        dev->clkm_conf.clkm_div_b = 0;

        // We ignore the possibility for fractional division here.
        // N = ( f_APLL_CLK / f_I2Sn_CLK ) - b/a
        dev->clkm_conf.clkm_div_num = ( (uint32_t)apll_freq / config->clock_speed_hz ) - ( dev->clkm_conf.clkm_div_b / dev->clkm_conf.clkm_div_a );

        ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Resulting I2S%d_CLK frequency should be %fHz.",
                        i2s_getDevNum( dev ),
                        apll_freq / (float)((float)dev->clkm_conf.clkm_div_num + (float)(dev->clkm_conf.clkm_div_b)/(dev->clkm_conf.clkm_div_a)) );
    }
    else {
        ESP_LOGW( TAG, "i2s_parallel_setup(): Failed to calculate APLL coefficients. Proceeding with PLL_D2_CLK as clock source." );
    */
        // Reset I2S clock configuration
        dev->clkm_conf.val = 0;

        dev->clkm_conf.clka_en = 0;     // Select PLL_D2_CLK as clock source (160MHz clock divided by 2)
        dev->clkm_conf.clkm_div_a = 1;
        dev->clkm_conf.clkm_div_b = 0;

        // We ignore the possibility for fractional division here.
        // N = ( PLL_D2_CLK / f_I2Sn_CLK ) - b/a
        // Default value of PLL_D2_CLK is 160MHz / 2 = 80MHz.
        dev->clkm_conf.clkm_div_num = 80000000L/config->clock_speed_hz;

        ESP_LOGD( TAG, "i2s_parallel_setup(): Setting I2S clock source: Resulting I2S%d_CLK frequency should be %fHz.",
                        i2s_getDevNum( dev ),
                        (float)( 80000000.0 / (dev->clkm_conf.clkm_div_num + (dev->clkm_conf.clkm_div_b)/(dev->clkm_conf.clkm_div_a)) ) );
    /*
    }
    */

    ESP_LOGD( TAG, "i2s_parallel_setup(): I2S clock configured with:" );
    ESP_LOGD( TAG, "i2s_parallel_setup(): \t clka_en = %d, clkm_div_a = %d, clkm_div_b = %d, clkm_div_num = %d,",
                    dev->clkm_conf.clka_en, dev->clkm_conf.clkm_div_a, dev->clkm_conf.clkm_div_b, dev->clkm_conf.clkm_div_num );
    ESP_LOGD( TAG, "i2s_parallel_setup(): \t tx_bck_div_num = %d, rx_bck_div_num = %d.", dev->sample_rate_conf.tx_bck_div_num, dev->sample_rate_conf.rx_bck_div_num );
    
    dev->fifo_conf.val = 0;
    dev->fifo_conf.rx_fifo_mod_force_en = 1; // From datasheet: "The bit should always be set to 1" but why?
    dev->fifo_conf.tx_fifo_mod_force_en = 1; // Datasheet: both these bits should be set to 1
    dev->fifo_conf.rx_fifo_mod = 1; // HN
    dev->fifo_conf.tx_fifo_mod = 1;     // Datasheet 12.4.4 Sending Data: Tx FIFO mode1 -
                                        // 16-bit single channel data; byte packing: 0A0B_0B0C = 0, 0A0B_0C0D = 1, 0A00_0B00 = 3
    dev->fifo_conf.rx_data_num = 32;    // These two control the length of the data that have been sent, received and buffered.
    dev->fifo_conf.tx_data_num = 32;    // FIFO length (in bits)
                                        // Doesn't make any difference in byte order when transmitting.
    dev->fifo_conf.dscr_en = 1;         // set this bit to enable I2S DMA mode (FIFO will use DMA descriptors)
	
	
    
    dev->conf1.val = 0;
    dev->conf1.tx_stop_en = 1;      // stop transmission when FIFO is empty
    dev->conf1.tx_pcm_bypass = 1;   // bypass compression module for the transmitted data


    
    dev->conf_chan.val = 0;
    dev->conf_chan.tx_chan_mod = 1;//config->tx_chan_mod; // Datasheet 12.4.4 Sending Data: mono mode:
                                    // When I2S_TX_MSB_RIGHT equals 0, the left-channel data are ”holding”
                                    // their values and the right-channel data change into the left-channel data.
    dev->conf_chan.rx_chan_mod = 1;//config->rx_chan_mod;
    
    // // Invert WS to be active-low... ToDo: make this configurable
    // dev->conf.tx_right_first = 1;   // "Set this bit to place right-channel data at the MSB in the transmit FIFO"
    // dev->conf.rx_right_first = 1;
    // Invert WS to be active-low... ToDo: make this configurable
    dev->conf.tx_right_first = 1;//config->tx_right_first;   // "Set this bit to place right-channel data at the MSB in the transmit FIFO"
    dev->conf.rx_right_first = 1;//config->rx_right_first;
    
    // !!! USEFUL !!!
    // Delay of clock line! Any many more...
    // See page 328 of the datasheet.
    dev->timing.val = 0;
    // dev->timing.val |= I2S_TX_BCK_OUT_DELAY_M;

    // From VGA controller source code:
    // clear serial mode flags
	dev->conf.tx_msb_right = 0;
    dev->conf.rx_msb_right = 0;
    // dev->conf.tx_msb_right = config->tx_msb_right;
    // dev->conf.rx_msb_right = config->rx_msb_right;

	dev->conf.tx_msb_shift = 0;
	dev->conf.tx_mono = 0;
	dev->conf.tx_short_sync = 0;
    
    // Allocate DMA descriptors...
    i2s_state[i2s_getDevNum(dev)] = malloc(sizeof(i2s_parallel_state_t));
    // dma_AllocateDescs( dev, config->buf );
    dma_AllocateDescs( dev );

    image_buf_desc = config->buf;
    outDataBuf_prepare();
    // i2s_state[i2s_getDevNum(dev)] = malloc(sizeof(i2s_parallel_state_t));
    // i2s_parallel_state_t *st = i2s_state[i2s_getDevNum(dev)];

    // st->dma_desc_count = calc_needed_dma_descs_count(config->buf);
	// ESP_LOGD( TAG,"i2s_parallel_setup(): number of descriptors = %d", st->dma_desc_count );
    // st->dma_desc_array = heap_caps_malloc( st->dma_desc_count*sizeof(lldesc_t), MALLOC_CAP_DMA );
    // // ... and fill them
    // init_dma_desc_array( st->dma_desc_array, config->buf, st->dma_desc_count ) ;


    // Reset FIFO/DMA -> needed? Doesn't dma_reset/fifo_reset do this?
    dev->lc_conf.in_rst = 1; 
	dev->lc_conf.out_rst = 1; 
	dev->lc_conf.ahbm_rst = 1; 
	dev->lc_conf.ahbm_fifo_rst = 1;
    dev->lc_conf.in_rst = 0; 
	dev->lc_conf.out_rst = 0; 
	dev->lc_conf.ahbm_rst = 0; 
	dev->lc_conf.ahbm_fifo_rst = 0;
    dev->conf.tx_reset = 1; 
	dev->conf.tx_fifo_reset = 1; 
	dev->conf.rx_fifo_reset = 1;
    dev->conf.tx_reset = 0; 
	dev->conf.tx_fifo_reset = 0; 
	dev->conf.rx_fifo_reset = 0;

    // Start DMA on front buffer
    // I2S_OUT_DATA_BURST_EN    -   set this bit to transmit data in "burst mode", clear for "byte mode"
    // I2S_OUTDSCR_BURST_EN     -   set this bit to transfer outlink descriptor in "burst mode", clear for "byte mode"
    // I2S_OUT_EOF_MODE         -   DMA I2S_OUT_EOF_INT generation mode:
    //                                  1: When DMA has popped all data from the FIFO;
    //                                  0: When AHB has pushed all data to the FIFO.
    // I2S_OUT_AUTO_WRBACK      -   set to enable automatic outlink-writeback when all data in tx buffer has been transmitted

    // dev->lc_conf.val = I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN | I2S_OUT_DATA_BURST_EN;
    dev->lc_conf.val = 0x00;//I2S_OUT_EOF_MODE; // 0x00; //I2S_OUT_DATA_BURST_EN | I2S_OUTDSCR_BURST_EN;    // | I2S_OUT_EOF_MODE;

    // dev->out_link.addr = ((uint32_t)(&st->dma_desc_array[0]));
    // dev->out_link.start = 1;

    // // Start transmission
    // dev->conf.tx_start = 1;

    // allocate disabled i2s interrupt
	const int interrupt_source[] = {ETS_I2S0_INTR_SOURCE, ETS_I2S1_INTR_SOURCE};

    ESP_LOGD( TAG, "i2s_parallel_setup(): Setting interrupt for I2S%d: %d", i2s_getDevNum( dev ), interrupt_source[i2s_getDevNum( dev )] );

    int flags = ESP_INTR_FLAG_INTRDISABLED | ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM;
	// ESP_ERROR_CHECK( esp_intr_alloc( interrupt_source[i2s_getDevNum( dev )], flags, &i2s_isr, NULL, &i2s_intr_handle ) );
    esp_err_t ret = esp_intr_alloc( interrupt_source[i2s_getDevNum( dev )], flags, &i2s_isr, (i2s_dev_t*) dev, &i2s_intr_handle );
    ESP_ERROR_CHECK( ret );
    // ESP_ERROR_CHECK( esp_intr_alloc( interrupt_source[i2s_getDevNum( dev )], ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, &i2s_intrHandler, dev, &i2s_intr_handle ) );
    // i2s_intr_handler = &i2s_intrHandler;
    // ESP_ERROR_CHECK( esp_intr_alloc( interrupt_source[i2s_getDevNum( dev )], ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM, &i2s_isr, NULL, &i2s_intr_handle ) );

    // ESP_LOGI( TAG, "&i2s_isr = %p,\t&i2s_intr_handle = %p", &i2s_isr, &i2s_intr_handle );

    // ESP_ERROR_CHECK( esp_intr_enable( i2s_intr_handle ) );
}

// 
// Prepare I2S for a transmission of all image lines.
// dev  - pointer to I2S device that is in use
// 
void i2s_prepareTx( i2s_dev_t *dev ){
// void i2s_send_buf( i2s_dev_t *dev ){
    
    ESP_ERROR_CHECK( esp_intr_disable( i2s_intr_handle ) );
    i2s_reset( dev );
    // i2s_stop( dev );
    dev->conf.tx_start = 0;
    dev->conf.rx_start = 0;

    int no = i2s_getDevNum(dev);
    if (i2s_state[no]==NULL){
        ESP_LOGW( TAG, "i2s_prepareTx(): i2s_state[%d] == NULL. Returning.", no );
        return;
    }

    // i2s_parallel_state_t *st = i2s_state[no];
    
    // if( st->dma_desc_array == NULL )
        // ESP_LOGI( TAG, "Descriptor empty!" );
    // init_dma_desc_array( st->dma_desc_array, data_buf, st->dma_desc_count ) ;

    // Loop the last descriptor back to the first one
    // Get the first dma_desc_array
    // lldesc_t *active_dma_chain;
    // active_dma_chain = (lldesc_t*) &i2s_state[no]->dma_desc_array[0];
    // Get the last dma_desc_array and set its next pointer to the first descriptor
    // i2s_state[no]->dma_desc_array[i2s_state[no]->dma_desc_count-1].qe.stqe_next = active_dma_chain;

    if( i2s_state[no]->dma_desc_array == NULL ){
        ESP_LOGW( TAG, "i2s_state[no]->dma_desc_array == NULL" );
        return;
    }

    active_dma_desc_idx = 0;

    // Set the DMA active buffer/descriptor to the first one 
    dev->out_link.addr = ((uint32_t)(&i2s_state[no]->dma_desc_array[0]));
    dev->out_link.start = 1;                // and start DMA link

    dev->int_clr.val = dev->int_raw.val;    // clear interrupt flags
	dev->int_ena.val = 0;                   // disable all I2S interrupts except...
	dev->int_ena.out_eof = 1;               // when finished sending a (FIFO or data?) packet and
	dev->int_ena.out_dscr_err = 1;          // when invalid DMA descriptors are encountered

    // int_ena.out_eof  causes an interrupt after sending 2 bytes of data (MSB of FIFO)

    ESP_ERROR_CHECK( esp_intr_enable( i2s_intr_handle ) );

    // // Start transmission
    // dev->conf.tx_start = 1;
}

// 
// Start I2S transmission.
// dev  - pointer to I2S device that is in use
// 
void i2s_startTx( i2s_dev_t *dev ){
    // Start transmission
    dev->conf.tx_start = 1;
}