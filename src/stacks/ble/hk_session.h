#pragma once

#include <host/ble_hs.h>

#include "../../common/hk_characteristics_properties.h"

typedef struct
{
    char transaction_id;
    char last_opcode;
    const void* static_data;
    char service_index;
    char service_id;
    const ble_uuid128_t* service_uuid;
    char characteristic_index;
    hk_characteristic_types_t characteristic_type;
} hk_session_t;