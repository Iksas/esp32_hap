#include "hk_chr_timed_write.h"

#include <time.h>
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"
#include "../../../common/hk_errors.h"

#include "../hk_formats_ble.h"

hk_mem *hk_chr_timed_write_write_request;
time_t time_of_death = -1;

esp_err_t hk_chr_timed_write(hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t res = ESP_OK;
    uint8_t ttl = 0;
    hk_mem *ttl_mem = hk_mem_create();
    hk_tlv_t *tlv_data = hk_tlv_deserialize(transaction->request);
    hk_chr_timed_write_write_request = hk_mem_create();

    if (hk_tlv_get_mem_by_type(tlv_data, 0x01, hk_chr_timed_write_write_request) != HK_RES_OK)
    {
        HK_LOGE("Error getting value of write request.");
        res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
    }

    if (res == ESP_OK)
    {
        if (hk_tlv_get_mem_by_type(tlv_data, 0x08, ttl_mem) != HK_RES_OK)
        {
            HK_LOGE("Error getting value of write request.");
            res = ESP_ERR_HK_UNSUPPORTED_REQUEST;
        }
    }

    if (res == ESP_OK)
    {
        ttl = *(uint8_t *)ttl_mem->ptr;
        time_t now;
        time(&now);
        time_of_death = now + ttl;
        HK_LOGD("Timed write requested. Now: %ld, Time to live: %d, Time of death: %ld", now, ttl, time_of_death);
    }

    hk_mem_free(ttl_mem);
    return res;
}

esp_err_t hk_chr_execute_write(hk_connection_t *connection, hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t res = ESP_OK;
    hk_mem *write_response = hk_mem_create();
    time_t now;
    time(&now);

    if (now > time_of_death)
    {
        HK_LOGD("Time of death: %ld, Now: %ld", time_of_death, now);
        HK_LOGE("Execute of timed write could not be done, as time was over.");
        res = ESP_ERR_NOT_SUPPORTED;
    }
    else
    {
        HK_LOGD("Executing timed write: Time of death: %ld, Now: %ld", time_of_death, now);
    }

    if (res == ESP_OK)
    {
        if (chr->write_callback != NULL)
        {
            res = chr->write_callback(hk_chr_timed_write_write_request);
        }
        else if (chr->write_with_response_callback)
        {
            res = chr->write_with_response_callback(connection, hk_chr_timed_write_write_request, write_response);
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

    hk_mem_free(write_response);
    hk_mem_free(hk_chr_timed_write_write_request);
    return res;
}