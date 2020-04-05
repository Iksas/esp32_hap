#pragma once

#include <freertos/FreeRTOS.h>
#include "../include/hk_mem.h"
#include "hk_connection.h"

esp_err_t hk_pairing_ble_write_pair_setup(hk_connection_t *connection, hk_mem* request, hk_mem* response);
esp_err_t hk_pairing_ble_write_pair_verify(hk_connection_t *connection, hk_mem* request, hk_mem* response);
esp_err_t hk_pairing_ble_read_pairing_features(hk_mem* response);
esp_err_t hk_pairing_ble_read_pairing_pairings(hk_mem* response);
esp_err_t hk_pairing_ble_write_pairing_pairings(hk_connection_t *connection, hk_mem* request, hk_mem* response);