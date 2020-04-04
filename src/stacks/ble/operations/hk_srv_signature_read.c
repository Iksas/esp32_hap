#include "hk_srv_signature_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

enum hk_srv_signature_read_srv_properties
{
    HK_SRV_PROP_PRIMARY = 0x0001,
    HK_SRV_PROP_HIDDEN = 0x0002,
    HK_SRV_PROP_SUPPORTS_CONFIGURATION = 0x0004
};

esp_err_t hk_srv_signature_read_response(const ble_uuid128_t *chr_uuid, hk_session_t *session)
{
    uint16_t properties = 0;
    if (session->srv_primary)
    {
        properties |= HK_SRV_PROP_PRIMARY;
    }

    if (session->srv_hidden)
    {
        properties |= HK_SRV_PROP_HIDDEN;
    }

    if (session->srv_supports_configuration)
    {
        properties |= HK_SRV_PROP_SUPPORTS_CONFIGURATION;
    }

    hk_tlv_t *tlv_data = NULL;
    tlv_data = hk_tlv_add_buffer(tlv_data, 0x0f,
                                 (void *)&properties, sizeof(uint16_t));

    tlv_data = hk_tlv_add_buffer(tlv_data, 0x10,
                                 NULL, 0);

    hk_tlv_serialize(tlv_data, session->response);

    return ESP_OK;
}