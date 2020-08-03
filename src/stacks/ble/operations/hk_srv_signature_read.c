#include "hk_srv_signature_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

enum hk_srv_signature_read_srv_properties //todo: as defines
{
    HK_SRV_PROP_PRIMARY = 0x0001,
    HK_SRV_PROP_HIDDEN = 0x0002,
    HK_SRV_PROP_SUPPORTS_CONFIGURATION = 0x0004
};

esp_err_t hk_srv_signature_read(hk_transaction_t *transaction, hk_chr_t *chr)
{
    uint16_t properties = 0;
    if (chr->srv_primary)
    {
        properties |= HK_SRV_PROP_PRIMARY;
    }

    if (chr->srv_hidden)
    {
        properties |= HK_SRV_PROP_HIDDEN;
    }

    if (chr->srv_supports_configuration)
    {
        properties |= HK_SRV_PROP_SUPPORTS_CONFIGURATION;
    }

    hk_tlv_t *tlv_data_response = NULL;
    tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x0f, (void *)&properties, sizeof(uint16_t));
    tlv_data_response = hk_tlv_add_buffer(tlv_data_response, 0x10, NULL, 0);

    hk_tlv_serialize(tlv_data_response, transaction->response);
    hk_tlv_free(tlv_data_response);

    return ESP_OK;
}