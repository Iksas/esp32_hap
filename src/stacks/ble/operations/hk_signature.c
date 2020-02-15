#include "hk_signature.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

void hk_signature_response(const ble_uuid128_t *characteristic_uuid, hk_session_t *session, hk_mem* response)
{
    hk_tlv_t *tlv_data = NULL;
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_CHRARACTERISTIC_TYPE, (char*)characteristic_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_SERVICE_ID, session->service_id);
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_SERVICE_TYPE, (char*)session->service_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_CHARACTERISTIC_PROPERTIES, 0x1);
    tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_USER_DESCRIPTION, "Version");
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_PRESENTATION_FORMAT, hk_formats_ble_get(session->characteristic_type), 7);
    // Characteristic Presentation Format: 0x19: string; 0x00: no exponent; 0x2700: unit = unitless; 0x01: Bluetooth SIG namespace; 0x0000: No description
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_VALID_RANGE, HK_CHR_VERSION);
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_STEP_VALUE, HK_CHR_VERSION);
    //size_t tlv_size = hk_tlv_get_size(tlv_data);

    hk_tlv_serialize(tlv_data, response);
}