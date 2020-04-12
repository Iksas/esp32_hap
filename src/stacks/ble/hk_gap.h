#pragma once

//#include <stdlib.h>
#include <stdbool.h>
#include <host/ble_hs.h>

void hk_gap_init(const char *name, size_t category, size_t config_version);
void hk_gap_set_address(uint8_t own_addr_type);
void hk_gap_start_advertising();
void hk_advertising_update_paired();
void hk_gap_terminate_connection(uint16_t connection_handle);