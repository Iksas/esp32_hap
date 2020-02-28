#pragma once

#include <freertos/FreeRTOS.h>
#include "../include/hk_mem.h"

void hk_pairing_ble_read_pair_setup();
void hk_pairing_ble_write_pair_setup(hk_mem* request, hk_mem* response);
void hk_pairing_ble_read_pair_verify(hk_mem* response);
void hk_pairing_ble_write_pair_verify(hk_mem* request, hk_mem* response);
void hk_pairing_ble_read_pairing_features(hk_mem* response);
void hk_pairing_ble_read_pairing_pairings(hk_mem* response);
void hk_pairing_ble_write_pairing_pairings(hk_mem* request, hk_mem* response);