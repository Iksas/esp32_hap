#include "hk_uuid_manager.h"

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"

ble_uuid128_t *hk_uuid_manager_uuids;

ble_uuid128_t* hk_uuid_manager_get(uint8_t id) {
    ble_uuid128_t* result_uuid = NULL;

    hk_ll_foreach(hk_uuid_manager_uuids, uuid) {
        if(uuid->value[12] == id)
        {
            result_uuid = uuid;
            //todo break;
        }
    }

    if(result_uuid == NULL){
        const uint8_t buffer[16] = {0x91, 0x52, 0x76, 0xbb, 0x26, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 
            id, 0x00, 0x00, 0x00};
        hk_uuid_manager_uuids = hk_ll_new(hk_uuid_manager_uuids);
        ble_uuid_init_from_buf((ble_uuid_any_t*)hk_uuid_manager_uuids, (const void*)buffer, 16);
        result_uuid = hk_uuid_manager_uuids;
    }

    return result_uuid;
}