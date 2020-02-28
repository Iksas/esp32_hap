#include "hk_service_signature_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

enum hk_service_signature_read_chr_properties
{
    CHARACTERISTIC_SUPPORTS_READ = 0x0001,
    CHARACTERISTIC_SUPPORTS_WRITE = 0x0002,
    CHARACTERISTIC_SUPPORTS_ADDITONAL_AUTHORIZATION_DATA = 0x0004,
    CHARACTERISTIC_REQUIRES_TIMED_WRITE = 0x0008,
    CHARACTERISTIC_SUPPORTS_SECURE_READ = 0x0010,
    CHARACTERISTIC_SUPPORTS_SECURE_WRITE = 0x0020,
    CHARACTERISTIC_HIDDEN_FROM_USER = 0x0040,
    CHARACTERISTIC_NOTIFIES_EVENTS_CONNECTED_STATE = 0x0080,
    CHARACTERISTIC_NOTIFIES_EVENTS_DISCONNECTED_STATE = 0x0100,
    CHARACTERISTIC_SUPPORTS_BROADCAST_NOTIFICATION = 0x0200
};

void hk_service_signature_read_response(const ble_uuid128_t *chr_uuid, hk_session_t *session, hk_mem* response)
{
    uint16_t properties = CHARACTERISTIC_SUPPORTS_ADDITONAL_AUTHORIZATION_DATA;
    hk_tlv_t *tlv_data = NULL;
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_HAP_SERVICE_PROPERTIES, 
        (void*)&properties, sizeof(uint16_t));

    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_HAP_LINKED_SERVICES, 
        NULL, 0);

    hk_tlv_serialize(tlv_data, response);
}