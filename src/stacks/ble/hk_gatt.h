#pragma once

#include "../../include/homekit_chrs.h"
#include "../../include/homekit_srvs.h"
#include "../../include/homekit_chrs.h"

#include <stdlib.h>
#include <stdbool.h>

#include "host/ble_gatt.h"

void hk_gatt_init();
void hk_gatt_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden);
void* hk_gatt_add_chr(hk_chr_types_t chr_type, void *(*read)(size_t*), void *(*write)(void *, size_t, size_t*), 
    bool can_notify, int16_t min_length, int16_t max_length);
void hk_gatt_add_chr_static_read(hk_chr_types_t type, const char *value);
void hk_gatt_end_config();
void hk_gatt_start();