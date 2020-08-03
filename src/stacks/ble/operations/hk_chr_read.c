#include "hk_chr_read.h"

#include "../../../include/hk_mem.h"
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

esp_err_t hk_chr_read(hk_transaction_t *transaction, hk_chr_t *chr)
{
    esp_err_t res = ESP_OK;
    hk_tlv_t *tlv_data_response = NULL;
    hk_mem *read_response = hk_mem_init();

    if (chr->static_data != NULL)
    {
        hk_mem_append_string(read_response, chr->static_data);
    }
    else
    {
        res = chr->read_callback(read_response);
    }

    if (res == ESP_OK)
    {
        if (read_response->size > 0)
        {
            tlv_data_response = hk_tlv_add_mem(tlv_data_response, 0x01, read_response);
        }

        hk_tlv_serialize(tlv_data_response, transaction->response);
    }

    hk_tlv_free(tlv_data_response);
    hk_mem_free(read_response);

    return res;
}