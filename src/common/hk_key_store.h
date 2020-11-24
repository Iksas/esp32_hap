/**
 * @file hk_key_store.h
 *
 * Stores the accessory keys.
 */

#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

esp_err_t hk_key_store_priv_get(hk_mem *value);
esp_err_t hk_key_store_priv_set(hk_mem *value);
esp_err_t hk_key_store_pub_get(hk_mem *value);
esp_err_t hk_key_store_pub_set(hk_mem *value);
