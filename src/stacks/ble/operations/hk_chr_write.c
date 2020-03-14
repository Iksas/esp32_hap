#include "hk_chr_write.h"

#include "../../../include/hk_mem.h"
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

void hk_chr_write_response(const ble_uuid128_t *chr_uuid, hk_session_t *session)
{
    hk_tlv_t *tlv_data = hk_tlv_deserialize(session->request);

    hk_mem *write_request = hk_mem_create();
    hk_mem *write_response = hk_mem_create();

    if (hk_tlv_get_mem_by_type(tlv_data, HK_TLV_VALUE, write_request) == HK_RES_OK)
    {
        session->write_callback(write_request, write_response);

        if (write_response->size > 0)
        {
            hk_tlv_t *tlv_data_response = NULL;
            tlv_data_response = hk_tlv_add_mem(tlv_data_response, HK_TLV_VALUE, write_response);
            hk_tlv_serialize(tlv_data_response, session->response);
        }
    }
    else
    {
        HK_LOGE("Error getting value of write request.");
        //todo: error handling
    }

    hk_mem_free(write_request);
    hk_mem_free(write_response);
}