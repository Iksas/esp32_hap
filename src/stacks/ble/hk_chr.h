#pragma once

#include <host/ble_hs.h>

#include "../../include/hk_mem.h"
#include "../../common/hk_chrs_properties.h"

#include "hk_connection.h"

typedef struct
{
    int srv_index;
    char srv_id;
    int chr_index;
    uint16_t instance_id;
    bool srv_primary;
    bool srv_hidden;
    bool srv_supports_configuration;
} hk_chr_setup_info_t;

typedef struct
{
    const char* static_data;
    esp_err_t (*read_callback)(hk_mem* response);
    esp_err_t (*write_callback)(hk_mem* request);
    esp_err_t (*write_with_response_callback)(hk_connection_t *connection, hk_mem *request, hk_mem *response);
    char srv_index;
    char srv_id;
    bool srv_primary;
    bool srv_hidden;
    bool srv_supports_configuration;
    const ble_uuid128_t* srv_uuid;
    const ble_uuid128_t* uuid;
    uint16_t chr_index;
    hk_chr_types_t chr_type;
    float max_length;
    float min_length;
} hk_chr_t;

hk_chr_t* hk_chr_init(hk_chr_types_t chr_type, hk_chr_setup_info_t *hk_gatt_setup_info);