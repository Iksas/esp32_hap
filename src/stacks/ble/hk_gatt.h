#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <esp_err.h>

#include "host/ble_gatt.h"

#include "../../include/hk_chrs.h"
#include "../../include/hk_srvs.h"
#include "../../include/hk_chrs.h"
#include "../../include/hk_mem.h"

#include "hk_connection.h"

void hk_gatt_init();
void hk_gatt_add_srv(
    hk_srv_types_t srv_type, 
    bool primary, 
    bool hidden, 
    bool supports_configuration);
esp_err_t hk_gatt_add_chr(
    hk_chr_types_t chr_type, 
    esp_err_t (*read)(hk_mem* response),
    esp_err_t (*write)(hk_mem* request), 
    esp_err_t (*write_with_response)(hk_connection_t *connection, hk_mem *request, hk_mem *response),
    bool can_notify, 
    float min_length, 
    float max_length,
    void** chr_ptr);
void hk_gatt_add_chr_static_read(hk_chr_types_t type, const char *value);
void hk_gatt_end_config();
void hk_gatt_start();
esp_err_t hk_gatt_indicate(void *ble_chr);