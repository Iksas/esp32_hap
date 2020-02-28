#include "hk_chr_signature_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../common/hk_chrs_properties.h"

#include "../hk_formats_ble.h"

void hk_chr_signature_read_response(const ble_uuid128_t *chr_uuid, hk_session_t *session, hk_mem* response)
{
    hk_tlv_t *tlv_data = NULL;
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_CHRARACTERISTIC_TYPE, (char*)chr_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_SERVICE_ID, session->srv_id);
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_SERVICE_TYPE, (char*)session->srv_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_CHARACTERISTIC_PROPERTIES, 
        hk_chrs_properties_get_prop(session->chr_type));
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_USER_DESCRIPTION, "Version");
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_PRESENTATION_FORMAT, hk_formats_ble_get(session->chr_type), 7);
    // Chr Presentation Format: 0x19: string; 0x00: no exponent; 0x2700: unit = unitless; 0x01: Bluetooth SIG namespace; 0x0000: No description
    if(session->min_length >= 0 || session->max_length >= 0){
        int16_t min_length = 0;
        if(session->min_length >= 0){
            min_length = session->min_length;
        }

        int16_t max_length = 0;
        if(session->max_length >= 0){
            max_length = session->max_length;
        }

        int16_t range[2];
        range[0] = min_length;
        range[1] = max_length;
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALID_RANGE, (void*)range, sizeof(int16_t) * 2);
    }
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_STEP_VALUE, HK_CHR_VERSION);
    //size_t tlv_size = hk_tlv_get_size(tlv_data);

    hk_tlv_serialize(tlv_data, response);
}