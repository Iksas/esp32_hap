#include "hk_chr_write.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

void hk_chr_write_response(const ble_uuid128_t *chr_uuid, hk_session_t *session, hk_mem* response)
{
    hk_tlv_t *tlv_data = hk_tlv_deserialize(session->request);
    hk_tlv_t *value_tlv = hk_tlv_get_tlv_by_type(tlv_data, HK_TLV_VALUE);

    size_t response_size = -1;
    void* response_data = session->write_callback(value_tlv->value, value_tlv->length, &response_size);
    
    if(response_size > 0) {
        hk_tlv_t *tlv_data_response = NULL;
        tlv_data_response = hk_tlv_add_buffer(tlv_data_response, HK_TLV_VALUE, (char *)response_data, response_size);
        hk_tlv_serialize(tlv_data_response, response);
    }
}