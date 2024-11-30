/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_check.h"
#include "rlcd_gsp_encoder.h"

static const char *TAG = "rlcd_gsp_encoder";

typedef struct {
    rmt_encoder_t base;           // the base "class", declares the standard encoder interface
    // rmt_encoder_t *copy_encoder;  // use the copy_encoder to encode the leading and ending pulse
    rmt_encoder_t *bytes_encoder; // use the bytes_encoder to encode the address and command data
    // rmt_symbol_word_t rlcd_gsp_leading_symbol;  // GSP leading code with RMT representation
                                                // used to sync GSP with RGB data pulses (delay GSP)
    // rmt_symbol_word_t rlcd_gsp_ending_symbol;   // GSP ending code with RMT representation
                                                // allows flexibility of the last GSP period,
                                                // after transmission of which, an interrupt
                                                // pulling INTB low can be triggered
    int state;
} rmt_rlcd_gsp_encoder_t;

static size_t rmt_encode_rlcd_gsp(rmt_encoder_t *encoder, rmt_channel_handle_t channel, const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state) {
    rmt_rlcd_gsp_encoder_t *gsp_encoder = __containerof(encoder, rmt_rlcd_gsp_encoder_t, base);
    rmt_encode_state_t session_state = 0;
    rmt_encode_state_t state = 0;
    size_t encoded_symbols = 0;
    rlcd_gsp_scan_code_t *scan_code = (rlcd_gsp_scan_code_t *)primary_data;
    // rmt_encoder_handle_t copy_encoder = gsp_encoder->copy_encoder;
    rmt_encoder_handle_t bytes_encoder = gsp_encoder->bytes_encoder;

    encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, &scan_code->data, sizeof(uint8_t), &session_state);
    if (session_state & RMT_ENCODING_COMPLETE) {
        gsp_encoder->state = 0; // back to the initial encoding session
        state |= RMT_ENCODING_COMPLETE;
    }
    if (session_state & RMT_ENCODING_MEM_FULL) {
        state |= RMT_ENCODING_MEM_FULL;
        goto out; // yield if there's no free space to put other encoding artifacts
    }

    // switch (gsp_encoder->state) {
    //     case 0: // send leading code
    //         encoded_symbols += copy_encoder->encode(copy_encoder, channel, &gsp_encoder->rlcd_gsp_leading_symbol,
    //                                                 sizeof(rmt_symbol_word_t), &session_state);
    //         if (session_state & RMT_ENCODING_COMPLETE) {
    //             gsp_encoder->state = 1; // we can only switch to next state when current encoder finished
    //         }
    //         if (session_state & RMT_ENCODING_MEM_FULL) {
    //             state |= RMT_ENCODING_MEM_FULL;
    //             goto out; // yield if there's no free space to put other encoding artifacts
    //         }
    //     // fall-through
    //     case 1: // send address
    //         encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, &scan_code->data, sizeof(uint8_t), &session_state);
    //         if (session_state & RMT_ENCODING_COMPLETE) {
    //             gsp_encoder->state = 2; // we can only switch to next state when current encoder finished
    //         }
    //         if (session_state & RMT_ENCODING_MEM_FULL) {
    //             state |= RMT_ENCODING_MEM_FULL;
    //             goto out; // yield if there's no free space to put other encoding artifacts
    //         }
    //     // fall-through
    //     case 2: // send ending code
    //         encoded_symbols += copy_encoder->encode(copy_encoder, channel, &gsp_encoder->rlcd_gsp_ending_symbol,
    //                                                 sizeof(rmt_symbol_word_t), &session_state);
    //         if (session_state & RMT_ENCODING_COMPLETE) {
    //             gsp_encoder->state = 0; // back to the initial encoding session
    //             state |= RMT_ENCODING_COMPLETE;
    //         }
    //         if (session_state & RMT_ENCODING_MEM_FULL) {
    //             state |= RMT_ENCODING_MEM_FULL;
    //             goto out; // yield if there's no free space to put other encoding artifacts
    //         }
    // }
out:
    *ret_state = state;
    return encoded_symbols;
}

static esp_err_t rmt_del_rlcd_gsp_encoder(rmt_encoder_t *encoder) {
    rmt_rlcd_gsp_encoder_t *gsp_encoder = __containerof(encoder, rmt_rlcd_gsp_encoder_t, base);
    // rmt_del_encoder(gsp_encoder->copy_encoder);
    rmt_del_encoder(gsp_encoder->bytes_encoder);
    free(gsp_encoder);
    return ESP_OK;
}

static esp_err_t rmt_rlcd_gsp_encoder_reset(rmt_encoder_t *encoder) {
    rmt_rlcd_gsp_encoder_t *gsp_encoder = __containerof(encoder, rmt_rlcd_gsp_encoder_t, base);
    // rmt_encoder_reset(gsp_encoder->copy_encoder);
    rmt_encoder_reset(gsp_encoder->bytes_encoder);
    gsp_encoder->state = 0;
    return ESP_OK;
}

esp_err_t rmt_new_rlcd_gsp_encoder(const rlcd_gsp_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder) {
    esp_err_t ret = ESP_OK;
    rmt_rlcd_gsp_encoder_t *gsp_encoder = NULL;
    ESP_GOTO_ON_FALSE(config && ret_encoder, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    gsp_encoder = calloc(1, sizeof(rmt_rlcd_gsp_encoder_t));
    ESP_GOTO_ON_FALSE(gsp_encoder, ESP_ERR_NO_MEM, err, TAG, "no mem for rLCD GSP encoder");
    gsp_encoder->base.encode = rmt_encode_rlcd_gsp;
    gsp_encoder->base.del = rmt_del_rlcd_gsp_encoder;
    gsp_encoder->base.reset = rmt_rlcd_gsp_encoder_reset;

    // rmt_copy_encoder_config_t copy_encoder_config = {};
    // ESP_GOTO_ON_ERROR(rmt_new_copy_encoder(&copy_encoder_config, &gsp_encoder->copy_encoder), err, TAG, "create copy encoder failed");

    // Leading and ending symbols: additional (dummy) GSP pulses as in datasheet of the rLCD.

    // construct the leading code and ending code with RMT symbol format
    // gsp_encoder->rlcd_gsp_leading_symbol = (rmt_symbol_word_t) {
    //     .level0 = 0,
    //     .duration0 = 18,//9,//100ULL * config->resolution / 10000000,//9000ULL * config->resolution / 1000000,    // xULL is in [us]
    //     .level1 = 0,
    //     .duration1 = 18,//9,//100ULL * config->resolution / 10000000,//4500ULL * config->resolution / 1000000,
    // };
    // gsp_encoder->rlcd_gsp_ending_symbol = (rmt_symbol_word_t) {
    //     .level0 = 1,
    //     .duration0 = 1,//100ULL * config->resolution / 10000000,//560 * config->resolution / 1000000,
    //     .level1 = 0,
    //     .duration1 = 1,//100ULL * config->resolution / 10000000,//0x7FFF,
    // };

    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 0,
            .duration0 = 191,//400ULL * config->resolution / 10000000,//560 * config->resolution / 1000000, // T0H=560us
            .level1 = 0,
            .duration1 = 1,//400ULL * config->resolution / 10000000,//560 * config->resolution / 1000000, // T0L=560us
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = 1024,//254,//32,//100ULL * config->resolution / 10000000,//560 * config->resolution / 1000000,  // T1H=560us
            .level1 = 0,
            .duration1 = 8,//254,//32,//100ULL * config->resolution / 10000000,//1690 * config->resolution / 1000000, // T1L=1690us
        },
    };
    ESP_GOTO_ON_ERROR(rmt_new_bytes_encoder(&bytes_encoder_config, &gsp_encoder->bytes_encoder), err, TAG, "create bytes encoder failed");

    *ret_encoder = &gsp_encoder->base;
    return ESP_OK;
err:
    if (gsp_encoder) {
        if (gsp_encoder->bytes_encoder) {
            rmt_del_encoder(gsp_encoder->bytes_encoder);
        }
        // if (gsp_encoder->copy_encoder) {
        //     rmt_del_encoder(gsp_encoder->copy_encoder);
        // }
        free(gsp_encoder);
    }
    return ret;
}