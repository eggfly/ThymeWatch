#ifndef _RLCD_GSP_ENCODER_H_
#define _RLCD_GSP_ENCODER_H_
/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once

#include <stdint.h>
#include "driver/rmt_encoder.h"

/**
 * @brief rLCD GSP scan code representation
 */
typedef struct {
    // uint32_t data;  // contains GSP (clock) signal periods
    uint8_t data;  // contains GSP (clock) signal periods
} rlcd_gsp_scan_code_t;

/**
 * @brief Type of rLCD GSP encoder configuration
 */
typedef struct {
    uint32_t resolution; /*!< Encoder resolution, in Hz */
} rlcd_gsp_encoder_config_t;

/**
 * @brief Create RMT encoder for encoding GSP signal into RMT symbols
 *
 * @param[in] config Encoder configuration
 * @param[out] ret_encoder Returned encoder handle
 * @return
 *      - ESP_ERR_INVALID_ARG for any invalid arguments
 *      - ESP_ERR_NO_MEM out of memory when creating IR NEC encoder
 *      - ESP_OK if creating encoder successfully
 */
esp_err_t rmt_new_rlcd_gsp_encoder(const rlcd_gsp_encoder_config_t *config, rmt_encoder_handle_t *ret_encoder);

#endif // _RLCD_GSP_ENCODER_H_