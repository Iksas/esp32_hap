#include "hk_pairing_ble.h"

#include "../../utils/hk_tlv.h"
#include "../../utils/hk_mem.h"
#include "../../common/hk_pair_setup.h"

const uint8_t hk_paring_ble_features = 0; //zero because non mfi certified
hk_mem *hk_pairing_ble_request = NULL;
hk_mem *hk_pairing_ble_response = NULL;
hk_mem *hk_pairing_ble_device_id = NULL;

#include "../../utils/hk_logging.h"

void *hk_pairing_ble_read_pair_setup(size_t* response_length)
{
    HK_LOGE("hk_pairing_ble_read_pair_setup");
    *response_length = 0;
    return NULL;
}

void *hk_pairing_ble_write_pair_setup(void *request, size_t request_length, size_t* response_length)
{    
    hk_pairing_ble_request = hk_mem_create(); // todo: never freed, but should be. Thinkabout a homekit data structure as public interface
    hk_pairing_ble_response = hk_mem_create();
    hk_pairing_ble_device_id = hk_mem_create();
    hk_mem_append_buffer(hk_pairing_ble_request, request, request_length);
    
    int res = hk_pair_setup(hk_pairing_ble_request, hk_pairing_ble_response, hk_pairing_ble_device_id);
    if(res != 0){
        HK_LOGE("Error in pair setup: %d", res);
    }

    *response_length = hk_pairing_ble_response->size;
    return hk_pairing_ble_response->ptr;
}

void *hk_pairing_ble_read_pair_verify(size_t* response_length)
{
    HK_LOGE("hk_pairing_ble_read_pair_verify");
    *response_length = 0;
    return NULL;
}

void *hk_pairing_ble_write_pair_verify(void *request, size_t request_length, size_t* response_length)
{
    HK_LOGE("hk_pairing_ble_write_pair_verify");
    *response_length = 0;
    return NULL;
}

void *hk_pairing_ble_read_pairing_features(size_t* response_length)
{
    *response_length = sizeof(uint8_t);
    return (void*)&hk_paring_ble_features;
}

void *hk_pairing_ble_read_pairing_pairings(size_t* response_length)
{
    HK_LOGE("hk_pairing_ble_read_pairing_pairings");
    *response_length = 0;
    return NULL;
}

void *hk_pairing_ble_write_pairing_pairings(void *request, size_t request_length, size_t* response_length)
{
    HK_LOGE("hk_pairing_ble_write_pairing_pairings");
    *response_length = 0;
    return NULL;
}