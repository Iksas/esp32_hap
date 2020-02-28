#include "hk_characteristic_read.h"

#include "../../../utils/hk_logging.h"
#include "../../../utils/hk_tlv.h"

#include "../hk_formats_ble.h"

void hk_characteristic_read_response(const ble_uuid128_t *characteristic_uuid, hk_session_t *session, hk_mem *response)
{
    hk_tlv_t *tlv_data = NULL;
    void *value = NULL;
    size_t size = -1;

    if (session->static_data != NULL)
    {
        size = strlen(session->static_data);
        value = (void*)session->static_data;
    }
    else
    {
        value = session->read_callback(&size);
    }

    if(size > 0){
        tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_VALUE, (char *)value, size);     
    }

    hk_tlv_serialize(tlv_data, response);
}