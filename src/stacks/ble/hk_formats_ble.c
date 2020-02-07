#include "hk_formats_ble.h"

#include "../../utils/hk_logging.h"

const char hk_formats_ble_bool[7] = {0x01, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_uint8[7] = {0x04, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_uint16[7] = {0x06, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_uint32[7] = {0x08, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_uint64[7] = {0x0A, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_int[7] = {0x10, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_float[7] = {0x14, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_string[7] = {0x19, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};
const char hk_formats_ble_data[7] = {0x1B, 0x00, 0x00, 0x27, 0x01, 0x00, 0x00};

char* hk_formats_ble_get(hk_characteristic_types_t characteristic_type)
{
    hk_format_t characteristic_format = hk_characteristics_properties_get_type(characteristic_type);
    switch(characteristic_format)
    {
        case HK_FORMAT_BOOL:
            return (char*)hk_formats_ble_bool;
        case HK_FORMAT_UINT8:
            return (char*)hk_formats_ble_uint8;
        case HK_FORMAT_UINT16:
            return (char*)hk_formats_ble_uint16;
        case HK_FORMAT_UINT32:
            return (char*)hk_formats_ble_uint32;
        case HK_FORMAT_UINT64:
            return (char*)hk_formats_ble_uint64;
        case HK_FORMAT_INT:
            return (char*)hk_formats_ble_int;
        case HK_FORMAT_FLOAT:
            return (char*)hk_formats_ble_float;
        case HK_FORMAT_STRING:
            return (char*)hk_formats_ble_string;
        case HK_FORMAT_TLV8:
        case HK_FORMAT_DATA:
            return (char*)hk_formats_ble_data;
        default:
            HK_LOGE("Cannot convert, because format not known: %d", characteristic_type);
    return NULL;
    }
    
}