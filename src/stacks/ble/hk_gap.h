#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <host/ble_hs.h>

void hk_gap_init(const char *name, size_t category, size_t config_version);
void hk_gap_start_advertising(uint8_t own_addr_type);
void hk_advertising_update_paired(bool initial);
