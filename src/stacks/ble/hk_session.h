#pragma once

#include <host/ble_hs.h>

#include "../../utils/hk_mem.h"
#include "../../common/hk_characteristics_properties.h"

typedef struct
{
    uint8_t transaction_id;
    uint8_t last_opcode;
    const char* static_data;
    void *(*read_callback)(size_t*);
    void *(*write_callback)(void *, size_t, size_t*);
    char service_index;
    char service_id;
    const ble_uuid128_t* service_uuid;
    char characteristic_index;
    hk_characteristic_types_t characteristic_type;
    int16_t max_length;
    int16_t min_length;
    hk_mem *request;
} hk_session_t;