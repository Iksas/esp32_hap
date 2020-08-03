#include "hk_chr_signature_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../common/hk_chrs_properties.h"

#include "../hk_formats_ble.h"

esp_err_t hk_chr_signature_read(const ble_uuid128_t *chr_uuid, hk_transaction_t *transaction, hk_chr_t *chr)
{
    hk_tlv_t *tlv_data_response = NULL;
    tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x04, (char *)chr_uuid->value, 16);      // chr type
    tlv_data_response = hk_tlv_add_uint16(tlv_data_response, 0x07, chr->srv_id);                      // srv id
    tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x06, (char *)chr->srv_uuid->value, 16); //srv type
    tlv_data_response = hk_tlv_add_uint16(tlv_data_response, 0x0a, hk_chrs_properties_get_prop(chr->chr_type)); // chr properties
    tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0c, hk_formats_ble_get(chr->chr_type), 7); // chr gatt format

    hk_format_t chr_format = hk_chrs_properties_get_type(chr->chr_type);

    switch (chr_format)
    {
    case HK_FORMAT_UINT8:
    {
        uint8_t range[2];
        range[0] = (uint8_t)chr->min_length;
        range[1] = (uint8_t)chr->max_length;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0d, (void *)range, sizeof(uint8_t) * 2);
        break;
    }
    case HK_FORMAT_UINT16:
    {
        uint16_t range[2];
        range[0] = (uint16_t)chr->min_length;
        range[1] = (uint16_t)chr->max_length;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0d, (void *)range, sizeof(uint16_t) * 2);
        break;
    }
    case HK_FORMAT_UINT32:
    {
        uint32_t range[2];
        range[0] = (uint32_t)chr->min_length;
        range[1] = (uint32_t)chr->max_length;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0d, (void *)range, sizeof(uint32_t) * 2);
        break;
    }
    case HK_FORMAT_UINT64:
    {
        uint64_t range[2];
        range[0] = (uint64_t)chr->min_length;
        range[1] = (uint64_t)chr->max_length;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0d, (void *)range, sizeof(uint64_t) * 2);
        break;
    }
    case HK_FORMAT_INT:
    {
        int32_t range[2];
        range[0] = (int32_t)chr->min_length;
        range[1] = (int32_t)chr->max_length;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0d, (void *)range, sizeof(int32_t) * 2);
        break;
    }
    case HK_FORMAT_FLOAT:
    {
        float range[2];
        range[0] = chr->min_length;
        range[1] = chr->max_length;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0d, (void *)range, sizeof(float) * 2);
        break;
    }
    case HK_FORMAT_STRING:
    case HK_FORMAT_TLV8:
    case HK_FORMAT_DATA:;
    case HK_FORMAT_BOOL:
        break;
    default:
        HK_LOGE("Found unknown characteristic format: %d", chr_format);
        break;
    }
    //todo: step value is missing
    hk_tlv_serialize(tlv_data_response, transaction->response);
    hk_tlv_free(tlv_data_response);
    return ESP_OK;
}