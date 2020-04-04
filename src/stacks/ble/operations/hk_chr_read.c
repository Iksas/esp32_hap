#include "hk_chr_read.h"

#include "../../../include/hk_mem.h"
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

esp_err_t hk_chr_read_response(const ble_uuid128_t *chr_uuid, hk_session_t *session)
{
    esp_err_t res = ESP_OK;
    hk_tlv_t *tlv_data = NULL;
    hk_mem *read_response = hk_mem_create();

    if (session->static_data != NULL)
    {
        hk_mem_append_string(read_response, session->static_data);
    }
    else
    {
        res = session->read_callback(read_response);
    }

    if (res == ESP_OK)
    {
        if (read_response->size > 0)
        {
            tlv_data = hk_tlv_add_mem(tlv_data, 0x01, read_response);
        }

        hk_tlv_serialize(tlv_data, session->response);
    }

    hk_mem_free(read_response);

    return res;
}