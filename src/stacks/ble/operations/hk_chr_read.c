#include "hk_chr_read.h"

#include "../../../include/hk_mem.h"
#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

void hk_chr_read_response(const ble_uuid128_t *chr_uuid, hk_session_t *session)
{
    hk_tlv_t *tlv_data = NULL;
    hk_mem* read_response = hk_mem_create();

    if (session->static_data != NULL)
    {
        hk_mem_append_string(read_response, session->static_data);
    }
    else
    {
        session->read_callback(read_response);
    }

    if(read_response->size > 0){
        tlv_data = hk_tlv_add_mem(tlv_data, HK_TLV_VALUE, read_response);     
    }

    hk_tlv_serialize(tlv_data, session->response);
    hk_mem_free(read_response);
}