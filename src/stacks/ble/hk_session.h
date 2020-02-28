#pragma once

#include <host/ble_hs.h>

#include "../../include/hk_mem.h"
#include "../../common/hk_chrs_properties.h"

typedef struct
{
    uint8_t transaction_id;
    uint8_t last_opcode;
    const char* static_data;
    void (*read_callback)(hk_mem* response);
    void (*write_callback)(hk_mem* request, hk_mem* response);
    char srv_index;
    char srv_id;
    const ble_uuid128_t* srv_uuid;
    char chr_index;
    hk_chr_types_t chr_type;
    int16_t max_length;
    int16_t min_length;
    hk_mem *request;
} hk_session_t;