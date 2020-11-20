#include "hk_chr_configuration.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

esp_err_t hk_chr_configuration(hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t ret = ESP_OK;
    hk_tlv_t *tlv_data_request = hk_tlv_deserialize(transaction->request);
    hk_tlv_log("hk_chr_configuration", tlv_data_request, true, true);

    hk_tlv_t *tlv_data_response = NULL;
    tlv_data_response = hk_tlv_add_uint16(tlv_data_response, 0x01, 1); // 1 = enable broadcast
    tlv_data_response = hk_tlv_add_uint8(tlv_data_response, 0x02, 1); // 1 = 20ms (default)
    
    hk_tlv_serialize(tlv_data_response, transaction->response);

    hk_tlv_log("hk_chr_configuration", tlv_data_response, true, true);
    hk_tlv_free(tlv_data_request);
    hk_tlv_free(tlv_data_response);
    return ret;
}