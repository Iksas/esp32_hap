#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

/**
 * @brief Returns the accessory id binary
 *
 * Returns the accessory id as buffer with 48 bit.
 *
 * @param id Memory to write the byte stream into.
 * 
 * @return Retruns ESP_OK if success.
 */
esp_err_t hk_accessory_id_get(hk_mem *id);

/**
 * @brief Returns the accessory id as string
 *
 * Returns the accessory id as string.
 *
 * @param id Memory to write the stream into.
 * 
 * @return Retruns ESP_OK if success.
 */
esp_err_t hk_accessory_id_get_serialized(hk_mem *id);

/**
 * @brief Resets the id
 *
 * Resets the id. It is recreated next time accessed.
 * 
 * @return Retruns ESP_OK if success.
 */
esp_err_t hk_accessory_id_reset();
