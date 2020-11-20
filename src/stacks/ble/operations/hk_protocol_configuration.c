#include "hk_protocol_configuration.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../utils/hk_util.h"
#include "../../../utils/hk_store.h"
#include "../../../crypto/hk_hkdf.h"
#include "../../../common/hk_global_state.h"
#include "../../../common/hk_accessory_id.h"
#include "../../../utils/hk_errors.h"

#include "../hk_formats_ble.h"
#include "../hk_connection_security.h"
#include "../hk_gap.h"
#include "../hk_broadcast_key.h"

esp_err_t hk_protocol_configuration(hk_conn_key_store_t *keys, hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t ret = ESP_OK;
    hk_tlv_t *tlv_data_request = hk_tlv_deserialize(transaction->request);

    bool is01 = hk_tlv_get_tlv_by_type(tlv_data_request, 0x01) != NULL;
    bool is02 = hk_tlv_get_tlv_by_type(tlv_data_request, 0x02) != NULL;
    if (is01 || is02)
    {
        HK_LOGW("hk_protocol_configuration 1");
        hk_mem *broadcast_key = hk_mem_init();
        hk_mem *accessory_id = hk_mem_init();

        if (is01) // reset broadcast key if requested
        {
            RUN_AND_CHECK(ret, hk_broadcast_key_reset, keys->accessory_shared_secret);
        }

        // get parameters
        uint16_t global_state = hk_global_state_get();
        uint8_t configuration = 0;
        RUN_AND_CHECK(ret, hk_store_u8_get, HK_STORE_CONFIGURATION, &configuration);
        RUN_AND_CHECK(ret, hk_accessory_id_get, accessory_id);
        RUN_AND_CHECK(ret, hk_broadcast_key_get, keys->accessory_shared_secret, broadcast_key);

        // generate response
        hk_tlv_t *tlv_data_response = NULL;
        tlv_data_response = hk_tlv_add_uint16(tlv_data_response, 0x01, global_state);
        tlv_data_response = hk_tlv_add_uint8(tlv_data_response, 0x02, configuration);
        tlv_data_response = hk_tlv_add_mem(tlv_data_response, 0x03, accessory_id);
        tlv_data_response = hk_tlv_add_mem(tlv_data_response, 0x04, broadcast_key);
        hk_tlv_serialize(tlv_data_response, transaction->response);

        hk_tlv_free(tlv_data_response);
        hk_mem_free(broadcast_key);
        hk_mem_free(accessory_id);
    }
    else
    {
        HK_LOGE("Error getting value of write request.");
        ret = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }

    hk_tlv_free(tlv_data_request);

    return ret;
}