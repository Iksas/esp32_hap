#include "hk_chr_write.h"

#include "../../../include/hk_mem.h"
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../common/hk_errors.h"

#include "../hk_formats_ble.h"

esp_err_t hk_chr_write(hk_connection_t *connection, hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t res = ESP_OK;
    hk_mem *write_request = hk_mem_create();
    hk_mem *write_response = hk_mem_create();
    hk_tlv_t *tlv_data = hk_tlv_deserialize(transaction->request);

    if (hk_tlv_get_mem_by_type(tlv_data, 0x01, write_request) != HK_RES_OK)
    {
        HK_LOGE("Error getting value of write request.");
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }

    if (res == ESP_OK)
    {
        if (chr->write_callback != NULL)
        {
            res = chr->write_callback(write_request);
        }
        else if (chr->write_with_response_callback)
        {
            res = chr->write_with_response_callback(connection, write_request, write_response);
        }
        else
        {
            HK_LOGE("Write callback was not found.");
            res = ESP_ERR_NOT_FOUND;
        }
    }

    if (res == ESP_OK)
    {
        if (write_response->size > 0)
        {
            hk_tlv_t *tlv_data_response = NULL;
            tlv_data_response = hk_tlv_add_mem(tlv_data_response, 0x01, write_response);
            hk_tlv_serialize(tlv_data_response, transaction->response);
        }
    }

    hk_mem_free(write_request);
    hk_mem_free(write_response);
    return res;
}