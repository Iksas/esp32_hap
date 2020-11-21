#include "hk_chr_write.h"

#include "../../../include/hk_mem.h"
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

esp_err_t hk_chr_write(hk_connection_t *connection, hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t ret = ESP_OK;
    hk_mem *write_request = hk_mem_init();
    hk_mem *write_response = hk_mem_init();
    hk_tlv_t *tlv_data_request = hk_tlv_deserialize(transaction->request);

    if (hk_tlv_get_mem_by_type(tlv_data_request, 0x01, write_request) != ESP_OK)
    {
        HK_LOGE("Error getting value of write request.");
        ret = ESP_ERR_INVALID_ARG;
    }

    if (ret == ESP_OK)
    {
        if (chr->write_callback != NULL)
        {
            ret = chr->write_callback(write_request);
        }
        else if (chr->write_with_response_callback)
        {
            ret = chr->write_with_response_callback(connection, write_request, write_response);
        }
        else
        {
            HK_LOGE("Write callback was not found.");
            ret = ESP_ERR_NOT_FOUND;
        }
    }

    if (ret == ESP_OK)
    {
        if (write_response->size > 0)
        {
            hk_tlv_t *tlv_data_response = NULL;
            tlv_data_response = hk_tlv_add_mem(tlv_data_response, 0x01, write_response);
            hk_tlv_serialize(tlv_data_response, transaction->response);
            hk_tlv_free(tlv_data_response);
        }
    }

    hk_mem_free(write_request);
    hk_mem_free(write_response);
    hk_tlv_free(tlv_data_request);
    return ret;
}