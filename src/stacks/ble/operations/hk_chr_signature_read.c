#include "hk_chr_signature_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../common/hk_chrs_properties.h"

#include "../hk_formats_ble.h"

void hk_chr_signature_read_response(const ble_uuid128_t *chr_uuid, hk_session_t *session)
{
    hk_tlv_t *tlv_data = NULL;
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_CHRARACTERISTIC_TYPE, (char *)chr_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_SERVICE_ID, session->srv_id);
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_SERVICE_TYPE, (char *)session->srv_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_CHARACTERISTIC_PROPERTIES,
                                 hk_chrs_properties_get_prop(session->chr_type));
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_USER_DESCRIPTION, "Version");
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_PRESENTATION_FORMAT, hk_formats_ble_get(session->chr_type), 7);

    hk_format_t chr_format = hk_chrs_properties_get_type(session->chr_type);

    switch (chr_format)
    {
    case HK_FORMAT_UINT8:
    {
        uint8_t range[2];
        range[0] = (uint8_t)session->min_length;
        range[1] = (uint8_t)session->max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void *)range, sizeof(uint8_t) * 2);
        break;
    }
    case HK_FORMAT_UINT16:
    {
        uint16_t range[2];
        range[0] = (uint16_t)session->min_length;
        range[1] = (uint16_t)session->max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void *)range, sizeof(uint16_t) * 2);
        break;
    }
    case HK_FORMAT_UINT32:
    {
        uint32_t range[2];
        range[0] = (uint32_t)session->min_length;
        range[1] = (uint32_t)session->max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void *)range, sizeof(uint32_t) * 2);
        break;
    }
    case HK_FORMAT_UINT64:
    {
        uint64_t range[2];
        range[0] = (uint64_t)session->min_length;
        range[1] = (uint64_t)session->max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void *)range, sizeof(uint64_t) * 2);
        break;
    }
    case HK_FORMAT_INT:
    {
        int32_t range[2];
        range[0] = (int32_t)session->min_length;
        range[1] = (int32_t)session->max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void *)range, sizeof(int32_t) * 2);
        break;
    }
    case HK_FORMAT_FLOAT:
    {
        float range[2];
        range[0] = session->min_length;
        range[1] = session->max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void *)range, sizeof(float) * 2);
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

    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_STEP_VALUE, HK_CHR_VERSION);
    //size_t tlv_size = hk_tlv_get_size(tlv_data);

    hk_tlv_serialize(tlv_data, session->response);
}