#pragma once

#include "../include/homekit_characteristics.h"

typedef enum
{
    HK_FORMAT_BOOL,
    HK_FORMAT_UINT8,
    HK_FORMAT_UINT16,
    HK_FORMAT_UINT32,
    HK_FORMAT_UINT64,
    HK_FORMAT_INT,
    HK_FORMAT_FLOAT,
    HK_FORMAT_STRING,
    HK_FORMAT_TLV8,
    HK_FORMAT_DATA,
    HK_FORMAT_UNKNOWN
} hk_format_t;

 hk_format_t hk_characteristics_properties_get_type(hk_characteristic_types_t characteristic_type);