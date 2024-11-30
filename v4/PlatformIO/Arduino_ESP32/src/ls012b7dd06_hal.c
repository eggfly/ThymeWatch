#include "ls012b7dd06_hal.h"
#include "ls012b7dd06.h"

#include "rom/ets_sys.h"    // for delay

#include "freertos/FreeRTOS.h"  // for task delay
#include "freertos/task.h"      // for task delay

#include "esp_log.h"

#include "driver/rmt_tx.h"
#include "rmt/rlcd_gck_encoder.h"
#include "rmt/rlcd_gsp_encoder.h"
#include "rmt/rlcd_intb_encoder.h"

#include "i2s_parallel_driver/i2s_parallel.h"

// #include "driver/ledc.h"        // for VCOM, VA and VB signals
// #include "driver/mcpwm.h"       // for VCOM, VA and VB signals
#include "driver/mcpwm_prelude.h"   // for VCOM, VA and VB signals

lcd_colour_t rlcd_buf[RLCD_BUF_SIZE] = {
    // .bits = (uint8_t) 0xff,
    // 0xff
    // 0x00
    //0x00, 0x00, // 2 dummy bits (clock edges) in front
    // 0x00, 0x00, 0xff,
    // 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80,
    // 0xff, 0x00, 0x00
    //0x00, 0x00  // 2 dummy bits (clock edges) at the end
};

static const char *TAG = "rlcd_lib";

void rlcd_setupPins( void ){
    // Setup GPIO pins

    // Additional for testing
    // gpio_reset_pin(GPIO_NUM_20);
    // gpio_set_direction(GPIO_NUM_20, GPIO_MODE_OUTPUT);

    // The following 4 are used for JTAG debugging by default
    gpio_reset_pin(RLCD_GSP);
    gpio_reset_pin(RLCD_GCK);
    gpio_reset_pin(RLCD_GEN);
    gpio_reset_pin(RLCD_INTB);
    gpio_reset_pin(RLCD_VB_VCOM);
    gpio_reset_pin(RLCD_VA);
    gpio_reset_pin(RLCD_BSP);
    gpio_reset_pin(RLCD_BCK);

    gpio_reset_pin(RLCD_R0);
    gpio_reset_pin(RLCD_R1);
    gpio_reset_pin(RLCD_G0);

    gpio_pullup_dis(RLCD_GSP);
    gpio_pullup_dis(RLCD_GCK);
    gpio_pullup_dis(RLCD_GEN);
    gpio_pullup_dis(RLCD_INTB);
    gpio_pullup_dis(RLCD_VB_VCOM);
    gpio_pullup_dis(RLCD_VA);
    gpio_pullup_dis(RLCD_BSP);
    gpio_pullup_dis(RLCD_BCK);

    gpio_set_direction(RLCD_GSP, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_GCK, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_GEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_INTB, GPIO_MODE_OUTPUT);

    gpio_set_direction(RLCD_VB_VCOM, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_VA, GPIO_MODE_OUTPUT);

    gpio_set_direction(RLCD_BSP, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_BCK, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_R0, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_R1, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_G0, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_G1, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_B0, GPIO_MODE_OUTPUT);
    gpio_set_direction(RLCD_B1, GPIO_MODE_OUTPUT);
}

void rlcd_setAllGPIOs( uint8_t pins_state ){
    gpio_set_level( RLCD_GSP, pins_state );
    gpio_set_level( RLCD_GCK, pins_state );
    gpio_set_level( RLCD_GEN, pins_state );
    gpio_set_level( RLCD_INTB, pins_state );

    gpio_set_level( RLCD_VB_VCOM, pins_state );
    gpio_set_level( RLCD_VA, pins_state );

    gpio_set_level( RLCD_BSP, pins_state );
    gpio_set_level( RLCD_BCK, pins_state );
    gpio_set_level( RLCD_R0, pins_state );
    gpio_set_level( RLCD_R1, pins_state );
    gpio_set_level( RLCD_G0, pins_state );
    gpio_set_level( RLCD_G1, pins_state );
    gpio_set_level( RLCD_B0, pins_state );
    gpio_set_level( RLCD_B1, pins_state );
}

void rlcd_testGPIOs( uint8_t pins_state ){
    rlcd_setAllGPIOs( pins_state );

    // Toggle state of all pins
    pins_state = !pins_state;
}

/* void rlcd_fillBlack( void ){
    // Begin frame
    gpio_set_level( RLCD_INTB, 1 );
    ets_delay_us( thsINTB );
    gpio_set_level( RLCD_GSP, 1 );
    ets_delay_us( thsGSP );

    // First dummy GCK high level
    gpio_set_level( RLCD_GCK, 1 );
    ets_delay_us( thwGCK );

    for( uint8_t line_num = 0; line_num < 240; line_num++ ){

        // Line feed starts with GCK low
        gpio_set_level( RLCD_GCK, 0 );

        // 
        // MSB data of a single line:
        // 

        gpio_set_level( RLCD_BSP, 1 );
        // ets_delay_ns( thsBSP );

        // First dummy high BCK level
        gpio_set_level( RLCD_BCK, 1 );
        // ets_delay_us( thwBCK );
        gpio_set_level( RLCD_BCK, 0 );
        // ets_delay_us( tlwBCK );

        gpio_set_level( RLCD_BSP, 0 );

        // Transmit 240 pixels in pairs, that is edges of BCK,
        // so 120 pairs (states of BCK),
        // so 60 periods of BCK
        for( uint8_t i = 0; i < 60 ; i++ ){

            // Pull GSP low in the middle of MSB of the 1st line.
            // GSP is pulled low in the middle of the first low GCK pulse
            // that is around transmission of the 120th pixel in MSB of
            // the 1st line (30th period of BCK).
            if( line_num == 0 && i == 30 )
                gpio_set_level( RLCD_GSP, 0 );

            // Here would be some code for pulling data from
            // the display buffer and setting RGB pins accordingly
            // on rising edges.

            gpio_set_level( RLCD_BCK, 1 );
            // ets_delay_us( thwBCK );

            // And here for data on falling edges.

            gpio_set_level( RLCD_BCK, 0 );
            // ets_delay_us( tlwBCK );
            
        }

        gpio_set_level( RLCD_BCK, 1 );
        // ets_delay_us( tlsGEN );

        // Toggle GEN for the last BCK cycle
        gpio_set_level( RLCD_GEN, 1 );

        // ets_delay_us( thwBCK );
        gpio_set_level( RLCD_BCK, 0 );

        ets_delay_us( thwGEN ); // actually should be (thwGEN - thwBCK)

        gpio_set_level( RLCD_GEN, 0 );
        ets_delay_us( thhGCK );


        // 
        // LSB data of a single line:
        // 

        gpio_set_level( RLCD_GCK, 1 );

        gpio_set_level( RLCD_BSP, 1 );
        // ets_delay_ns( thsBSP );

        gpio_set_level( RLCD_BCK, 1 );
        // ets_delay_us( thwBCK );
        gpio_set_level( RLCD_BCK, 0 );
        // ets_delay_us( tlwBCK );

        gpio_set_level( RLCD_BSP, 0 );

        // Transmit 240 pixels in pairs, that is edges of BCK,
        // so 120 pairs (states of BCK),
        // so 60 periods of BCK
        for( uint8_t i = 0; i < 60 ; i++ ){
            // Here would be some code for pulling data from
            // the display buffer and setting RGB pins accordingly.
            gpio_set_level( RLCD_BCK, 1 );
            // ets_delay_us( thwBCK );
            gpio_set_level( RLCD_BCK, 0 );
            // ets_delay_us( tlwBCK );
            
        }

        gpio_set_level( RLCD_BCK, 1 );
        // ets_delay_us( tlsGEN );

        // Toggle GEN for the last BCK cycle
        gpio_set_level( RLCD_GEN, 1 );

        // ets_delay_us( thwBCK );
        gpio_set_level( RLCD_BCK, 0 );

        ets_delay_us( thwGEN ); // actually should be (thwGEN - thwBCK)

        gpio_set_level( RLCD_GEN, 0 );
        ets_delay_us( thhGCK );
    }

    // Trailing dummy GCK period
    gpio_set_level( RLCD_GCK, 0 );
    ets_delay_us( tlwGCK );
    gpio_set_level( RLCD_GCK, 1 );
    ets_delay_us( thwGCK );
    gpio_set_level( RLCD_GCK, 0 );

    gpio_set_level( RLCD_INTB, 0 );
}
*/

/* All bits are '1's except of the first and the last one.
// This should give total of 62 periods of BCK (that is binary '1's),
// that is 2 dummy periods and 60 for RGB data.
// The first and the last bit can be used for time adjustment of
// the whole BCK signal.
// const rlcd_bck_scan_code_t bck_payload = {
//     .data = 0x7FFFFFFFFFFFFFFE
//     // .data = 0x7FFFFE
// };
*/

i2s_parallel_buffer_desc_t bufdesc;

// RMT stuff

rmt_channel_handle_t rmt_ch_array[3] = {NULL};

// rmt_channel_handle_t rmt_gck_channel = NULL;
rmt_encoder_handle_t rlcd_gck_encoder_handle = NULL;
rmt_encoder_handle_t rlcd_gsp_encoder_handle = NULL;
rmt_encoder_handle_t rlcd_intb_encoder_handle = NULL;

// All bits are '1's except of the first one.
// This should give total of 62 periods of BCK (that is binary '1's),
// that is 2 dummy periods and 60 for RGB data.
// The first and the last bit can be used for time adjustment of
// the whole BCK signal.
const rlcd_gck_scan_code_t gck_payload = {
    .data = { [ 0 ... 29 ] = 0b01111111 }
};
const rlcd_gsp_scan_code_t gsp_payload = {
    .data = 0b00000010
};
const rlcd_intb_scan_code_t intb_payload = {
    .data = 0b00000010
};
rmt_transmit_config_t transmit_config_common;

void rlcd_rmt_installEncoderGCK( rmt_channel_handle_t* rmt_ch_array ){
    ESP_LOGI(TAG, "Creating RMT TX channel for GCK: frequency = %dHz, mem_block_symbols = %d and trans_queue_depth = %d ...",
                  RLCD_GCK_FREQ, 64, 8 );
    rmt_tx_channel_config_t tx_channel_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT, //RMT_CLK_SRC_REF_TICK,    // 1MHz clock source //RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RLCD_GCK_FREQ,   // in Hz
        .mem_block_symbols = 64, // amount of RMT symbols that the channel can store at a time
        .trans_queue_depth = 8,  // number of transactions that allowed to pending in the background, this example won't queue multiple transactions, so queue depth > 1 is sufficient
        .gpio_num = RLCD_GCK,
    };
    // ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_gck_channel));
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_ch_array[0]));

    ESP_LOGI(TAG, "Setting modulate carrier to TX channel for GCK: duty_cycle = %f, frequency_hz = %dHz...",
                  (float)0.5, 0);
    rmt_carrier_config_t carrier_cfg = {
        .duty_cycle = 0.5,
        .frequency_hz = 0,  // no carrier frequency for digital transmittion
    };
    // ESP_ERROR_CHECK(rmt_apply_carrier(rmt_gck_channel, &carrier_cfg));
    ESP_ERROR_CHECK(rmt_apply_carrier(rmt_ch_array[0], &carrier_cfg));

    ESP_LOGI(TAG, "Installing encoder for GCK: resolution = %d...", RLCD_GCK_FREQ );
    rlcd_gck_encoder_config_t rlcd_gck_encoder_cfg = {
        .resolution = RLCD_GCK_FREQ,
    };
    ESP_ERROR_CHECK(rmt_new_rlcd_gck_encoder(&rlcd_gck_encoder_cfg, &rlcd_gck_encoder_handle));

    ESP_LOGI(TAG, "Enabling RMT TX channel for GCK...");
    // ESP_ERROR_CHECK(rmt_enable(rmt_gck_channel));
    ESP_ERROR_CHECK(rmt_enable(rmt_ch_array[0]));

    ESP_LOGI(TAG, "Done. RMT encoder for GCK successufully installed." );
}

void rlcd_rmt_installEncoderGSP( rmt_channel_handle_t* rmt_ch_array ){
    ESP_LOGI(TAG, "Creating RMT TX channel for GSP: frequency = %dHz, mem_block_symbols = %d and trans_queue_depth = %d ...",
                  RLCD_GSP_FREQ, 64, 8 );
    rmt_tx_channel_config_t tx_channel_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT, //RMT_CLK_SRC_REF_TICK,    // 1MHz clock source //RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RLCD_GSP_FREQ,   // in Hz
        .mem_block_symbols = 64, // amount of RMT symbols that the channel can store at a time
        .trans_queue_depth = 8,  // number of transactions that allowed to pending in the background, this example won't queue multiple transactions, so queue depth > 1 is sufficient
        .gpio_num = RLCD_GSP,
    };
    // ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_gsp_channel));
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_ch_array[1]));

    ESP_LOGI(TAG, "Setting modulate carrier to TX channel for GSP: duty_cycle = %f, frequency_hz = %dHz...",
                  (float)0.5, 0);
    rmt_carrier_config_t carrier_cfg = {
        .duty_cycle = 0.5,
        .frequency_hz = 0,  // no carrier frequency for digital transmittion
    };
    // ESP_ERROR_CHECK(rmt_apply_carrier(rmt_gsp_channel, &carrier_cfg));
    ESP_ERROR_CHECK(rmt_apply_carrier(rmt_ch_array[1], &carrier_cfg));

    ESP_LOGI(TAG, "Installing encoder for GSP: resolution = %d...", RLCD_GSP_FREQ );
    rlcd_gsp_encoder_config_t rlcd_gsp_encoder_cfg = {
        .resolution = RLCD_GSP_FREQ,
    };
    ESP_ERROR_CHECK(rmt_new_rlcd_gsp_encoder(&rlcd_gsp_encoder_cfg, &rlcd_gsp_encoder_handle));

    ESP_LOGI(TAG, "Enabling RMT TX channel for GSP...");
    // ESP_ERROR_CHECK(rmt_enable(rmt_gsp_channel));
    ESP_ERROR_CHECK(rmt_enable(rmt_ch_array[1]));

    ESP_LOGI(TAG, "Done. RMT encoder for GSP successufully installed." );
}

void rlcd_rmt_installEncoderINTB( rmt_channel_handle_t* rmt_ch_array ){
    ESP_LOGI(TAG, "Creating RMT TX channel for INTB: frequency = %dHz, mem_block_symbols = %d and trans_queue_depth = %d ...",
                  RLCD_INTB_FREQ, 64, 8 );
    rmt_tx_channel_config_t tx_channel_cfg = {
        .clk_src = RMT_CLK_SRC_DEFAULT, //RMT_CLK_SRC_REF_TICK,    // 1MHz clock source //RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RLCD_INTB_FREQ,   // in Hz
        .mem_block_symbols = 64, // amount of RMT symbols that the channel can store at a time
        .trans_queue_depth = 8,  // number of transactions that allowed to pending in the background, this example won't queue multiple transactions, so queue depth > 1 is sufficient
        .gpio_num = RLCD_INTB,
    };
    // ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_intb_channel));
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_channel_cfg, &rmt_ch_array[2]));

    ESP_LOGI(TAG, "Setting modulate carrier to TX channel for INTB: duty_cycle = %f, frequency_hz = %dHz...",
                  (float)0.5, 0);
    rmt_carrier_config_t carrier_cfg = {
        .duty_cycle = 0.5,
        .frequency_hz = 0,  // no carrier frequency for digital transmittion
    };
    // ESP_ERROR_CHECK(rmt_apply_carrier(rmt_intb_channel, &carrier_cfg));
    ESP_ERROR_CHECK(rmt_apply_carrier(rmt_ch_array[2], &carrier_cfg));

    ESP_LOGI(TAG, "Installing encoder for INTB: resolution = %d...", RLCD_INTB_FREQ );
    rlcd_intb_encoder_config_t rlcd_intb_encoder_cfg = {
        .resolution = RLCD_INTB_FREQ,
    };
    ESP_ERROR_CHECK(rmt_new_rlcd_intb_encoder(&rlcd_intb_encoder_cfg, &rlcd_intb_encoder_handle));

    ESP_LOGI(TAG, "Enabling RMT TX channel for INTB...");
    // ESP_ERROR_CHECK(rmt_enable(rmt_intb_channel));
    ESP_ERROR_CHECK(rmt_enable(rmt_ch_array[2]));

    ESP_LOGI(TAG, "Done. RMT encoder for INTB successufully installed." );
}

/* LEDC stuff
bool va_is_enabled = false;

ledc_channel_config_t ledc_ch_va = {
    .speed_mode     = LEDC_MODE,
    .channel        = LEDC_CHANNEL_0,
    .timer_sel      = LEDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = RLCD_VA,
    .duty           = 0, // Set duty to 0%
    .hpoint         = 0,
    .flags.output_invert = 0
};

ledc_channel_config_t ledc_ch_vb = {
    .speed_mode     = LEDC_MODE,
    .channel        = LEDC_CHANNEL_1,
    .timer_sel      = LEDC_TIMER,
    .intr_type      = LEDC_INTR_DISABLE,
    .gpio_num       = RLCD_VB_VCOM,
    .duty           = 0, // Set duty to 0%
    .hpoint         = 0,
    .flags.output_invert = 0
};

static void ledc_init( void ){
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer_va = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK( ledc_timer_config( &ledc_timer_va ) );

    ledc_timer_config_t ledc_timer_vb = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER_1,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 5 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK( ledc_timer_config( &ledc_timer_vb ) );

    // Prepare and then apply the LEDC PWM channel configuration
    // ledc_channel_config_t ledc_ch_va = {
    //     .speed_mode     = LEDC_MODE,
    //     .channel        = LEDC_CHANNEL,
    //     .timer_sel      = LEDC_TIMER,
    //     .intr_type      = LEDC_INTR_DISABLE,
    //     .gpio_num       = LEDC_OUTPUT_IO,
    //     .duty           = 0, // Set duty to 0%
    //     .hpoint         = 0,
    //     .flags.output_invert = 0
    // };
    ESP_ERROR_CHECK( ledc_channel_config( &ledc_ch_va ) );
    ESP_ERROR_CHECK( ledc_channel_config( &ledc_ch_vb ) );

    // Set duty to 50%
    ESP_ERROR_CHECK( ledc_set_duty( LEDC_MODE, LEDC_CHANNEL_0, LEDC_DUTY ) );
    // Update duty to apply the new value
    ESP_ERROR_CHECK( ledc_update_duty( LEDC_MODE, LEDC_CHANNEL_0 ) );

    ets_delay_us( 18 );

    ESP_ERROR_CHECK( ledc_set_duty( LEDC_MODE, LEDC_CHANNEL_1, LEDC_DUTY ) );
    ESP_ERROR_CHECK( ledc_update_duty( LEDC_MODE, LEDC_CHANNEL_1 ) );

    // // Initialize fade service.
    // ledc_fade_func_install(0);
    // ledc_cbs_t callbacks = {
    //     .fade_cb = cb_ledc_fade_end_event
    // };

    // SemaphoreHandle_t counting_sem = xSemaphoreCreateCounting(LEDC_TEST_CH_NUM, 0);

    // ledc_cb_register( ledc_ch_va.speed_mode, ledc_ch_va.channel, &callbacks, (void *)counting_sem );

}

void toggleVA( void ){
    while(1){
        if( va_is_enabled ){
            ESP_ERROR_CHECK( ledc_timer_pause( LEDC_MODE, LEDC_TIMER_0 ) );
            ESP_ERROR_CHECK( ledc_timer_rst( LEDC_MODE, LEDC_TIMER_0 ) );

            ets_delay_us( 18 );

            ESP_ERROR_CHECK( ledc_timer_pause( LEDC_MODE, LEDC_TIMER_1 ) );
            ESP_ERROR_CHECK( ledc_timer_rst( LEDC_MODE, LEDC_TIMER_1 ) );

            // ets_delay_us( 17 );
            // ESP_ERROR_CHECK( ledc_stop( LEDC_MODE, LEDC_CHANNEL_0, 0 ) );
            // ESP_ERROR_CHECK( ledc_stop( LEDC_MODE, LEDC_CHANNEL_1, 0 ) );

            // gpio_set_level( RLCD_VB_VCOM, 0 );
            // gpio_set_level( RLCD_VA, 0 );

            va_is_enabled = false;
        }
        else {
            ESP_ERROR_CHECK( ledc_timer_resume( LEDC_MODE, LEDC_TIMER_0 ) );

            ets_delay_us( 18 );

            ESP_ERROR_CHECK( ledc_timer_resume( LEDC_MODE, LEDC_TIMER_1 ) );

            // ESP_ERROR_CHECK( ledc_set_duty( LEDC_MODE, LEDC_CHANNEL_0, LEDC_DUTY ) );
            // ESP_ERROR_CHECK( ledc_set_duty( LEDC_MODE, LEDC_CHANNEL_1, LEDC_DUTY ) );
            // ESP_ERROR_CHECK( ledc_update_duty( LEDC_MODE, LEDC_CHANNEL_0 ) );
            // ESP_ERROR_CHECK( ledc_update_duty( LEDC_MODE, LEDC_CHANNEL_1 ) );

            // ESP_ERROR_CHECK( ledc_set_duty_and_update( LEDC_MODE, LEDC_CHANNEL_0, LEDC_DUTY, ledc_ch_va.hpoint ) );
            // ESP_ERROR_CHECK( ledc_set_duty_and_update( LEDC_MODE, LEDC_CHANNEL_1, LEDC_DUTY, ledc_ch_vb.hpoint ) );
            
            va_is_enabled = true;
        }

        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}
*/

/* Motor control legacy (deprecated)
void pwm_enable( void ){
    ESP_ERROR_CHECK( mcpwm_start( MCPWM_UNIT_0, MCPWM_TIMER_0 ) );
}

void pwm_disable( void ){
    ESP_ERROR_CHECK( mcpwm_stop( MCPWM_UNIT_0, MCPWM_TIMER_0 ) );
}

void va_vb_init( void ){
    mcpwm_gpio_init( MCPWM_UNIT_0, MCPWM0A, RLCD_VA ); // To drive a RC servo, one MCPWM generator is enough
    mcpwm_gpio_init( MCPWM_UNIT_0, MCPWM0B, RLCD_VB_VCOM );

    mcpwm_config_t pwm_config = {
        .frequency = 29, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,     // duty cycle of PWMxA = 0
        .cmpr_b = 50,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init( MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config );

    ESP_ERROR_CHECK( mcpwm_set_duty_type( MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0 ) );
    ESP_ERROR_CHECK( mcpwm_set_duty_type( MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_0 ) );

    ESP_ERROR_CHECK( mcpwm_set_duty( MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50 ) );
    ESP_ERROR_CHECK( mcpwm_set_duty( MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, 50 ) );

    pwm_enable();
}

bool pwm_is_enabled = false;

void togglePWM( void ){
    while(1){
        if( pwm_is_enabled ){
            pwm_disable();
            pwm_is_enabled = false;
        }
        else {
            pwm_enable();
            pwm_is_enabled = true;
        }

        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

*/

mcpwm_timer_handle_t pwm_timer_handle = NULL;
mcpwm_timer_handle_t pwm_timer_handle_VB = NULL;

mcpwm_gen_handle_t genr_a = NULL;
mcpwm_gen_handle_t genr_b = NULL;

bool pwm_is_enabled = false;

mcpwm_sync_handle_t timer_sync_handle;

// 
// Enable PWM signal generation.
// 
void pwm_enable( void ){
    ESP_ERROR_CHECK( mcpwm_timer_start_stop( pwm_timer_handle, MCPWM_TIMER_START_NO_STOP ) );
    vTaskDelay( 20 / portTICK_PERIOD_MS );
    ESP_ERROR_CHECK( mcpwm_timer_start_stop( pwm_timer_handle_VB, MCPWM_TIMER_START_NO_STOP ) );
    // ESP_ERROR_CHECK( mcpwm_soft_sync_activate( timer_sync_handle ) );
}

// 
// Disable PWM signal generation.
// 
void pwm_disable( void ){
    ESP_ERROR_CHECK( mcpwm_timer_start_stop( pwm_timer_handle_VB, MCPWM_TIMER_STOP_FULL ) );
    vTaskDelay( 20 / portTICK_PERIOD_MS );
    ESP_ERROR_CHECK( mcpwm_timer_start_stop( pwm_timer_handle, MCPWM_TIMER_STOP_FULL ) );
    // ESP_ERROR_CHECK( mcpwm_generator_set_force_level( genr_a, 0, false ) );
    // ESP_ERROR_CHECK( mcpwm_generator_set_force_level( genr_b, 0, false ) );
}

// 
// Configure MCPWM generators.
// gena, genb - MCPWM generator handles
// cmpa, cmpb - MCPWM comparator handles
// 
static void gen_action_config(mcpwm_gen_handle_t gena, mcpwm_gen_handle_t genb, mcpwm_cmpr_handle_t cmpa, mcpwm_cmpr_handle_t cmpb){
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gena,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gena,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(genb,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpb, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    
    /*
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gena,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpb, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));

    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpb, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(genb,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    */
    /*
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gena,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_HIGH),
                    // MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpb, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(gena,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_LOW),
                    // MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_TIMER_EVENT_ACTION_END()));

    // ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(genb,
    //                 // MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_LOW),
    //                 MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_HIGH),
    //                 MCPWM_GEN_TIMER_EVENT_ACTION_END()));
    ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpb, MCPWM_GEN_ACTION_HIGH),
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpa, MCPWM_GEN_ACTION_LOW),
                    MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    */

    // ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gena,
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_HIGH),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpb, MCPWM_GEN_ACTION_LOW),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    // ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpb, MCPWM_GEN_ACTION_HIGH),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpa, MCPWM_GEN_ACTION_LOW),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION_END()));


    // ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(gena,
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_HIGH),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpb, MCPWM_GEN_ACTION_LOW),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    // ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_compare_event(genb,
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, cmpa, MCPWM_GEN_ACTION_LOW),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, cmpb, MCPWM_GEN_ACTION_HIGH),
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION_END()));
    // ESP_ERROR_CHECK(mcpwm_generator_set_actions_on_timer_event(genb,
    //                 MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_LOW),
    //                 MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_DOWN, MCPWM_TIMER_EVENT_FULL, MCPWM_GEN_ACTION_HIGH),
    //                 MCPWM_GEN_TIMER_EVENT_ACTION_END()));
}

// 
// Configure and enable MCPWM timers to generate VA and VB/VCOM signals.
// 
void va_vb_init( void ){
    ESP_LOGI(TAG, "Create timer and operator");
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,   // 1us resolution
        .period_ticks = 40000,      // 40000us - 25Hz
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP//_DOWN,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &pwm_timer_handle));

    mcpwm_timer_config_t timer_config_VB = {
        .group_id = 1,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = 1000000,   // 1us resolution
        .period_ticks = 40000,      // 40000us - 25Hz
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP//_DOWN,
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config_VB, &pwm_timer_handle_VB));

    /* // Software sync; useless right now
    // mcpwm_soft_sync_config_t timer_sync_cfg = {
    // };
    // ESP_ERROR_CHECK( mcpwm_new_soft_sync_src( &timer_sync_cfg, &timer_sync_handle ) );
    */

    /* // Timer sync; works, but not for my needs
    // // Create a sync source object
    // mcpwm_timer_sync_src_config_t timer_sync_cfg = {
    //     .timer_event = MCPWM_TIMER_EVENT_FULL,
    //     .flags.propagate_input_sync = false
    // };
    // mcpwm_sync_handle_t timer_sync_src_handle;

    // ESP_ERROR_CHECK( mcpwm_new_timer_sync_src( pwm_timer_handle, &timer_sync_cfg, &timer_sync_src_handle ) );

    // // Set phase on sync
    // mcpwm_timer_sync_phase_config_t timer_sync_phase_cfg = {
    //     .sync_src = timer_sync_src_handle,
    //     .count_value = 0,
    //     .direction = MCPWM_TIMER_DIRECTION_UP
    // };

    // ESP_ERROR_CHECK( mcpwm_timer_set_phase_on_sync( pwm_timer_handle, &timer_sync_phase_cfg ) );
    */

    /* // GPIO sync; doesn't seem to work in this application
    // mcpwm_sync_handle_t gpio_sync_source = NULL;
    // mcpwm_gpio_sync_src_config_t gpio_sync_config = {
    //     .group_id = 0,              // GPIO fault should be in the same group of the above timers
    //     .gpio_num = RLCD_VA,
    //     .flags.pull_up = true,
    //     .flags.active_neg = true,  // By default, a posedge pulse can trigger a sync event
    //     .flags.io_loop_back = true
    // };
    // ESP_ERROR_CHECK( mcpwm_new_gpio_sync_src( &gpio_sync_config, &gpio_sync_source ) );

    // mcpwm_timer_sync_phase_config_t sync_phase_config = {
    //     .count_value = 0,                      // sync phase: target count value
    //     .direction = MCPWM_TIMER_DIRECTION_UP, // sync phase: count direction
    //     .sync_src = gpio_sync_source,          // sync source
    // };
    
    // ESP_ERROR_CHECK( mcpwm_timer_set_phase_on_sync( pwm_timer_handle, &sync_phase_config ) );
    */


    // MCPWM[MCPWM_UNIT_0]->int_ena.val = CAP0_INT_EN | CAP1_INT_EN | CAP2_INT_EN;  //Enable interrupt on  CAP0, CAP1 and CAP2 signal
    // mcpwm_isr_register(MCPWM_UNIT_0, isr_handler, NULL, ESP_INTR_FLAG_IRAM, NULL);  //Set ISR Handler

    // mcpwm_timer_event_callbacks_t cbs = {
    //     .on_full = 1,
    //     .on_empty = 1,
    // };

    // ESP_ERROR_CHECK( mcpwm_timer_register_event_callbacks( pwm_timer_handle, &cbs, &isr_handler ) );
    // ESP_ERROR_CHECK( mcpwm_timer_register_event_callbacks( pwm_timer_handle, &cbs, NULL ) );

    mcpwm_oper_handle_t oper = NULL;
    mcpwm_operator_config_t operator_config = {
        .group_id = 0, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &oper));

    mcpwm_oper_handle_t oper_VB = NULL;
    mcpwm_operator_config_t operator_config_VB = {
        .group_id = 1, // operator must be in the same group to the timer
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config_VB, &oper_VB));

    ESP_LOGI(TAG, "Connect timer and operator");
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper, pwm_timer_handle));
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(oper_VB, pwm_timer_handle_VB));

    ESP_LOGI(TAG, "Create cmpr_a and cmpr_b from the operator");
    mcpwm_cmpr_handle_t cmpr_a = NULL;
    mcpwm_cmpr_handle_t cmpr_b = NULL;

    mcpwm_comparator_config_t comparator_config = {
        .flags.update_cmp_on_tez = true,
    };

    ESP_ERROR_CHECK(mcpwm_new_comparator(oper, &comparator_config, &cmpr_a));
    ESP_ERROR_CHECK(mcpwm_new_comparator(oper_VB, &comparator_config, &cmpr_b));
    
    ESP_LOGI(TAG, "Create genr_a and genr_b from the operator");
    // mcpwm_gen_handle_t genr_a = NULL;
    // mcpwm_gen_handle_t genr_b = NULL;

    mcpwm_generator_config_t gen_a_config = {
        .gen_gpio_num = RLCD_VA,
    };
    mcpwm_generator_config_t gen_b_config = {
        .gen_gpio_num = RLCD_VB_VCOM,
    };
    
    ESP_ERROR_CHECK(mcpwm_new_generator(oper, &gen_a_config, &genr_a));
    ESP_ERROR_CHECK(mcpwm_new_generator(oper_VB, &gen_b_config, &genr_b));

    // set the initial compare value, so that the servo will spin to the center position
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_a, 20000));//10));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(cmpr_b, 20000));

    ESP_LOGI(TAG, "Set genr_a and genr_b action on timer and compare event");
    gen_action_config( genr_a, genr_b, cmpr_a, cmpr_b );
    // go high on counter empty
    // ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(generator,
    //                 MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // // go low on compare threshold
    // ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(generator,
    //                 MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, comparator, MCPWM_GEN_ACTION_LOW)));

    ESP_LOGI(TAG, "Enable and start timer");
    ESP_ERROR_CHECK(mcpwm_timer_enable(pwm_timer_handle));
    // ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer_handle, MCPWM_TIMER_START_NO_STOP));

    // vTaskDelay( 20 / portTICK_PERIOD_MS );
    ESP_ERROR_CHECK(mcpwm_timer_enable(pwm_timer_handle_VB));
    // ESP_ERROR_CHECK(mcpwm_timer_start_stop(pwm_timer_handle_VB, MCPWM_TIMER_START_NO_STOP));
}

// 
// Enable or disable generation of PWM signals.
// 
void togglePWM( void ){
    while(1){
        if( pwm_is_enabled ){
            pwm_disable();
            pwm_is_enabled = false;
        }
        else {
            pwm_enable();
            pwm_is_enabled = true;
        }

        vTaskDelay( 1000 / portTICK_PERIOD_MS );
    }
}

// 
// Fill the image buffer with black colour
// (clear the screen image buffer).
// 
void rlcd_fillImageBlack( void ){
    for( uint16_t i=0; i < RLCD_BUF_SIZE; i++ ){
        rlcd_buf[i].val = 0x00;
    }
}

// 
// Fill the image buffer with white colour.
// 
void rlcd_fillImageWhite( void ){
    for( uint32_t i=0; i < RLCD_DISP_W * RLCD_DISP_H; i++ ){
        rlcd_buf[i].val = 0xff;
    }
}

// 
// Fill the image buffer with given colour.
// colour - colour the whole image will be filled with,
//          has to be compatible with lcd_colour_t
// 
void rlcd_fillImageColour( uint8_t colour ){
    for( uint32_t i=0; i < RLCD_DISP_W * RLCD_DISP_H; i++ ){
        rlcd_buf[i].val = colour;
    }
}

// 
// Draw a single pixel.
// x, y - coordinates of the pixel,
//        signed int, to make drawing of big pictures easier
// colour - colour of the pixel,
//          has to be compatible with lcd_colour_t
// 
void rlcd_putPixel( int16_t x, int16_t y, uint8_t colour ){
    if( (x < 0) || (x > RLCD_DISP_W) || (y < 0) || (y > RLCD_DISP_H) )
        return;
    
    rlcd_buf[ x + (y * RLCD_DISP_W) ].val = colour;
}

// 
// Store data from the image buffer in the I2S data buffer.
// After calling this function, a new frame can be drawn
// on the image buffer.
void rlcd_updateImageBuf( void ){
    outDataBuf_update();
}

// 
// Turn on the LCD.
// 
void rlcd_resume( void ){
    // Power-on sequence:

    // Turn on voltage regulators
    // and wait for them to settle.

    // Wait for 2 GCK cycles after VDDs went HIGH
    // ets_delay_us( 267 );    //tlwGCK*2 + thwGCK*2 );

    // Pixel memory init: write all screen black.
    // rlcd_fillBlack();
    // rlcd_fillImageBlack();
    // rlcd_updateImageBuf( true );
    outDataBuf_clearImage();
    vTaskDelay( 10 / portTICK_PERIOD_MS );
    rlcd_sendFrame();

    // Wait for the end of the image transmission
    vTaskDelay( 40 / portTICK_PERIOD_MS );

    // Wait >=30us for VCOM, VA and VB
    // ets_delay_us( 30 );

    // Start VCOM, VA and VB

    pwm_enable();

    rlcd_updateImageBuf();

    vTaskDelay( 50 / portTICK_PERIOD_MS );
}

// 
// Shut down the LCD.
// 
void rlcd_suspend( void ){
    // Power off sequence:

    // Wait until the last frame transmission finishes,
    // just in case.
    vTaskDelay( 40 / portTICK_PERIOD_MS );

    // Pixel memory init: write all screen black.
    outDataBuf_clearImage();
    vTaskDelay( 10 / portTICK_PERIOD_MS );
    rlcd_sendFrame();

    vTaskDelay( 40 / portTICK_PERIOD_MS );

    pwm_disable();

    // Wait >=30us for VCOM, VA and VB
    // ets_delay_us( 30 );

    // Turn off voltage regulators

}

// 
// Initialize ESP32 peripherals and software
// to handle the LCD (not the display itself).
// 
void rlcd_init( void ){
    rlcd_setupPins();
    rlcd_setAllGPIOs( 0 );

    /* // For uint32_t buffer:

    // 1st transmitted byte octet
    rlcd_buf[0]  = 0xf0f2f4f8;         // 4th 3rd   \ 1st
    rlcd_buf[1]  = 0x01020408;         // 2nd 1st   /
    rlcd_buf[2]  = 0x10121418;         // 4th 3rd   \ 2nd
    rlcd_buf[3]  = 0x20222428;         // 2nd 1st   /
    
    // 2nd transmitted byte octet
    rlcd_buf[4]  = 0x40414248;                  // 3rd  
    rlcd_buf[5]  = 0x80818288;                  // 4th  
    rlcd_buf[6]  = 0x03060C18;  // 0b00000011   // 1st  
                                // 0b00000110
                                // 0b00001100
                                // 0b00011000
    rlcd_buf[7]  = 0x070E1C38;  // 0b00000111   // 2nd  
                                // 0b00001110
                                // 0b00011100
                                // 0b00111000
    */

    /* // For uint16_t buffer:

    // 1st transmitted byte octet
    //              MSB LSB <- MSB goes first from the FIFO
    rlcd_buf[0]  = 0xf0f1;         // 4th 3rd   \ 1st
    rlcd_buf[1]  = 0xf2f4;         // 2nd 1st   /
    rlcd_buf[2]  = 0xf801;         // 4th 3rd   \ 2nd
    rlcd_buf[3]  = 0x0204;         // 2nd 1st   /
    
    // 2nd transmitted byte octet
    rlcd_buf[4]  = 0x0810;         // 3rd  
    rlcd_buf[5]  = 0x1214;   // 4th  
    rlcd_buf[6]  = 0x1820;   // 1st  
    rlcd_buf[7]  = 0x2224;   // 2nd  

    // // 1st transmitted byte octet
    // rlcd_buf[0]  = 0x00f0;         // 4th 3rd   \ 1st
    // rlcd_buf[1]  = 0x00f1;         // 2nd 1st   /
    // rlcd_buf[2]  = 0x00f2;         // 4th 3rd   \ 2nd
    // rlcd_buf[3]  = 0x00f4;         // 2nd 1st   /
    
    // // 2nd transmitted byte octet
    // rlcd_buf[4]  = 0x00f8;         // 3rd  
    // rlcd_buf[5]  = 0x0001;   // 4th  
    // rlcd_buf[6]  = 0x0002;   // 1st  
    // rlcd_buf[7]  = 0x0004;   // 2nd  

    // // 1st transmitted byte octet
    // rlcd_buf[0]  = 0xf0f1;         // 4th 3rd   \ 1st
    // rlcd_buf[1]  = 0xf2f4;         // 2nd 1st   /
    // rlcd_buf[2]  = 0xf801;         // 4th 3rd   \ 2nd
    // rlcd_buf[3]  = 0x0204;         // 2nd 1st   /
    
    // // 2nd transmitted byte octet
    // rlcd_buf[4]  = 0x0810;         // 3rd  
    // rlcd_buf[5]  = 0x2040;   // 4th  
    // rlcd_buf[6]  = 0x8011;   // 1st  
    // rlcd_buf[7]  = 0x1214;   // 2nd  

    */

    /* // For uint8_t buffer:

    // Fill the LCD buffer
    // for( int i=0; i < (BUF_S-2)/8; i++ ){
    //     rlcd_buf[i*8+1] = 0x01;
    //     rlcd_buf[i*8+2] = 0x02;
    //     rlcd_buf[i*8+3] = 0x04;
    //     rlcd_buf[i*8+4] = 0x08;
    //     rlcd_buf[i*8+5] = 0x10;
    //     rlcd_buf[i*8+6] = 0x20;
    //     rlcd_buf[i*8+7] = 0x40;
    //     rlcd_buf[i*8+8] = 0x80;
    //     // rlcd_buf[i+1] = 1 << (i);
    // }

    // 1st transmitted byte quartet
    rlcd_buf[0]  = 0xf0;         // 3rd
    rlcd_buf[1]  = 0xf1;         // 4th
    rlcd_buf[2]  = 0xf2;         // 1st
    rlcd_buf[3]  = 0xf4;         // 2nd
                 //  TX chan mod     1    |   3   |
    // 2nd transmitted byte quartet
    rlcd_buf[4]  = 0xf8;         // 3rd  
    rlcd_buf[5]  = 0b00000001;   // 4th  
    rlcd_buf[6]  = 0b00000010;   // 1st  
    rlcd_buf[7]  = 0b00000100;   // 2nd  
    // 3rd transmitted byte quartet
    rlcd_buf[8]  = 0b00001000;   // 3rd  \ LSB
    rlcd_buf[9]  = 0b00010000;   // 4th  /    of FIFO which is 32-bit (4-byte) long
    rlcd_buf[10] = 0b00100000;   // 1st  \ MSB
    rlcd_buf[11] = 0b01000000;   // 2nd  /    of FIFO
    // 4th transmitted byte quartet etc.
    rlcd_buf[12] = 0b10000000;
    rlcd_buf[13] = 0b00010001;
    rlcd_buf[14] = 0b00010010;
    rlcd_buf[15] = 0b00010100;

    rlcd_buf[16] = 0b00011000;
    rlcd_buf[17] = 0b00010010;
    rlcd_buf[18] = 0b00100010;
    rlcd_buf[19] = 0b01000010;

    rlcd_buf[20] = 0b10000010;
    rlcd_buf[21] = 0b00100001;
    rlcd_buf[22] = 0b00100010;
    rlcd_buf[23] = 0b00100100;

    rlcd_buf[24] = 0b00101000;
    rlcd_buf[25] = 0b00110000;
    rlcd_buf[26] = 0b01100000;
    // rlcd_buf[27] = 0b01000000;
    // rlcd_buf[28] = 0b10000000;
    rlcd_buf[BUF_S-5] = 0xf0;

    rlcd_buf[BUF_S-4] = 0xf1;
    rlcd_buf[BUF_S-3] = 0x00;   // the last byte sent
    rlcd_buf[BUF_S-2] = 0xf4;
    rlcd_buf[BUF_S-1] = 0xf8;

    */

    // for( uint16_t i=0; i < RLCD_DISP_W; i++ ){
    //     rlcd_buf[i] = (1<<2);
    // }
    // for( uint16_t i=0; i < RLCD_DISP_W; i++ ){
    //     rlcd_buf[2*i] = (1<<3);
    // }
    rlcd_fillImageBlack();

    /*

    // for( uint16_t i=0; i < LINE_WIDTH; i++ ){
    //     rlcd_buf[i] = (1<<0);
    // }

    // for( uint16_t i=0; i < LINE_WIDTH; i++ ){
    //     rlcd_buf[LINE_WIDTH + i] = (1<<1);
    // }

    // for( uint16_t i=0; i < LINE_WIDTH; i++ ){
    //     rlcd_buf[LINE_WIDTH*2 + i] = (1<<2);
    // }

    // for( uint16_t i=0; i < LINE_WIDTH; i++ ){
    //     rlcd_buf[LINE_WIDTH*3 + i] = (1<<3);
    // }

    


    // rlcd_buf[1] = 0x01;
    // rlcd_buf[2] = 0x02;
    // rlcd_buf[3] = 0x04;
    // rlcd_buf[4] = 0x08;
    // rlcd_buf[5] = 0x01;
    // rlcd_buf[6] = 0x02;
    // rlcd_buf[7] = 0x04;
    // rlcd_buf[8] = 0x08;

    // 
    // I2S setup
    // 

    // i2s_parallel_buffer_desc_t bufdesc;

    // i2s_parallel_config_t cfg = {
    //     .gpio_bus = {
    //         RLCD_R0, 	// 0 : d0 
    //         RLCD_R1, 	// 1 : d1
    //         RLCD_G0, 	// 2 : d2
    //         RLCD_G1, 	// 3 : d3
    //         RLCD_B0, 	// 4 : d4
    //         RLCD_B1, 	// 5 : d5
    //         RLCD_BSP,	// 6 : d6 (HS?)
    //         RLCD_GEN	// 7 : d7 (VS?)
    //     },
    //     .gpio_clk = RLCD_BCK,

    //     .bits = I2S_PARALLEL_BITS_8,    // 8-bit mode (8 parallel output lines)
    //     .clock_speed_hz = 2*1000*1000,  // pixel clock frequency
    //     .buf = &bufdesc                 // image buffer
    // };
    */

    i2s_parallel_config_t cfg = {
        .gpio_bus = {
            RLCD_R0, 	// 0 : d0 
            RLCD_R1, 	// 1 : d1
            RLCD_G0, 	// 2 : d2
            RLCD_G1, 	// 3 : d3
            RLCD_B0, 	// 4 : d4
            RLCD_B1, 	// 5 : d5
            RLCD_BSP,	// 6 : d6
            RLCD_GEN	// 7 : d7
        },
        .gpio_clk = RLCD_BCK,

        .bits = I2S_PARALLEL_BITS_8,    // 8-bit mode (8 parallel output lines)
        .clock_speed_hz = 2*1250000,  // 5MHz pixel clock frequency
        .buf = &bufdesc                 // image buffer
    };

    // cfg->buf = &bufdesc;

    bufdesc.memory = rlcd_buf;
    bufdesc.size = RLCD_BUF_SIZE;

    vTaskDelay(50 / portTICK_PERIOD_MS);    // wait for voltage to stabilize (what voltage??)

    i2s_parallel_setup( &I2S1, &cfg );
    // i2s_parallel_setup( &I2S1, cfg );

    vTaskDelay(50 / portTICK_PERIOD_MS);

    // ESP_LOGD( TAG, "I2S init done with flags:\n tx_right_first=%d,\n rx_right_first=%d,\n tx_msb_right=%d,\n rx_msb_right=%d,\n tx_chan_mod=%d,\n rx_chan_mod=%d",
    //                 cfg.tx_right_first, cfg.rx_right_first, cfg.tx_msb_right, cfg.rx_msb_right, cfg.tx_chan_mod, cfg.rx_chan_mod );
    ESP_LOGD( TAG, "I2S init done with flags:\n tx_right_first=%d,\n rx_right_first=%d\n",
                    cfg.tx_right_first, cfg.rx_right_first );

    rlcd_rmt_installEncoderGCK( rmt_ch_array );
    rlcd_rmt_installEncoderGSP( rmt_ch_array );
    rlcd_rmt_installEncoderINTB( rmt_ch_array );

    // For VCOM, VA and VB signals
    // ledc_init();
    va_vb_init();
    
    // gpio_set_level( RLCD_INTB, 1 );

    // gpio_set_level( RLCD_INTB, 0 );


    rlcd_resume();

    // Normal operation:

    ESP_LOGI( TAG, "LCD init done." );

    // Send a frame
    // ets_delay_ms
}


// 
// Send one frame to the LCD, using ESP32's hardware.
// 
void rlcd_sendFrame( void ){
    
    i2s_prepareTx( &I2S1 );

    ESP_ERROR_CHECK( rmt_transmit( rmt_ch_array[2], rlcd_intb_encoder_handle, &intb_payload, sizeof(intb_payload), &transmit_config_common ) );
    ESP_ERROR_CHECK( rmt_transmit( rmt_ch_array[1], rlcd_gsp_encoder_handle, &gsp_payload, sizeof(gsp_payload), &transmit_config_common ) );
    ESP_ERROR_CHECK( rmt_transmit( rmt_ch_array[0], rlcd_gck_encoder_handle, &gck_payload, sizeof(gck_payload), &transmit_config_common ) );
    // i2s_setStopSignal();    // has to be before i2s_send_buf(), because I2S's ISR gets called earlier than any function called here I guess
    // i2s_send_buf( &I2S1, &bufdesc );

    ets_delay_us( 64 );

    i2s_startTx( &I2S1 );
    // i2s_send_buf( &I2S1 );

    // i2s_setStopSignal();
    
    // ets_delay_us( 30 );
    // i2s_stop( &I2S1 );
}