#pragma once

#include <freertos/FreeRTOS.h>

void *hk_pairing_ble_read_pair_setup();
void *hk_pairing_ble_write_pair_setup(void *new_value, size_t body_length, size_t* response_length);
void *hk_pairing_ble_read_pair_verify(size_t* response_length);
void *hk_pairing_ble_write_pair_verify(void *new_value, size_t body_length, size_t* response_length);
void *hk_pairing_ble_read_pairing_features(size_t* response_length);
void *hk_pairing_ble_read_pairing_pairings(size_t* response_length);
void *hk_pairing_ble_write_pairing_pairings(void *new_value, size_t body_length, size_t* response_length);