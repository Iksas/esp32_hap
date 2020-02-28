#pragma once

#include "../../include/homekit_characteristics.h"
#include "../../include/homekit_services.h"
#include "../../include/homekit_characteristics.h"

#include <stdlib.h>
#include <stdbool.h>

#include "host/ble_gatt.h"

void hk_gatt_init();
void hk_gatt_add_service(hk_service_types_t service_type, bool primary, bool hidden);
void* hk_gatt_add_characteristic(hk_characteristic_types_t characteristic_type, void *(*read)(size_t*), void *(*write)(void *, size_t, size_t*), 
    bool can_notify, int16_t min_length, int16_t max_length);
void hk_gatt_add_characteristic_static_read(hk_characteristic_types_t type, const char *value);
void hk_gatt_end_config();
void hk_gatt_start();