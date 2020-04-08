#include "hk_protocol_configuration.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../utils/hk_store.h"
#include "../../../crypto/hk_hkdf.h"
#include "../../../common/hk_global_state.h"
#include "../../../common/hk_errors.h"

#include "../hk_formats_ble.h"
#include "../hk_connection_security.h"

esp_err_t hk_protocol_configuration(hk_pair_verify_keys_t *keys, hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t res = ESP_OK;
    hk_tlv_t *tlv_data = hk_tlv_deserialize(transaction->request);

    if (hk_tlv_get_tlv_by_type(tlv_data, 0x01) != NULL)
    {
        // get public key and keys->shared_secret
        hk_mem *accessory_public_key = hk_mem_create();
        hk_store_key_pub_get(accessory_public_key);

        // generate broadcast_key
        hk_mem *broadcast_key = hk_mem_create();
        hk_hkdf_with_external_salt(keys->shared_secret, broadcast_key, accessory_public_key, HK_HKDF_BROADCAST_ENCRYPTION_KEY_INFO);

        // get global state number and current configuration
        uint16_t global_state = hk_global_state_get();
        uint8_t configuration = hk_store_configuration_get();

        // generate response
        hk_tlv_t *tlv_data_response = NULL;
        tlv_data_response = hk_tlv_add_uint16(tlv_data_response, 0x01, global_state);
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, 0x02, configuration);
        hk_tlv_serialize(tlv_data_response, transaction->response);

        hk_mem_free(accessory_public_key);
        hk_mem_free(broadcast_key);
        HK_LOGD("Request for generating broadcast encryption key. Done.");
    }
    else if (hk_tlv_get_tlv_by_type(tlv_data, 0x02) != NULL)
    {
        HK_LOGE("Request for getting all parameters.");
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }
    else
    {
        HK_LOGE("Error getting value of write request.");
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
        //todo: error handling
    }

    return res;
}