#pragma once

//#include <stdlib.h>
#include <stdbool.h>
#include <host/ble_hs.h>

#include "../../include/hk_mem.h"

void hk_gap_init(const char *name, size_t category, size_t config_version);
void hk_gap_address_set(uint8_t own_addr_type);
esp_err_t hk_gap_start_advertising();
esp_err_t hk_gap_start_advertising_change(uint16_t chr_index, hk_mem *value);
void hk_gap_terminate_connection(uint16_t handle);