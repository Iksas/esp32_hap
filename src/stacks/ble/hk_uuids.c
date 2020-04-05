#include "hk_uuids.h"

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"

ble_uuid128_t *hk_uuids_uuids;
char *hk_uuids_log_buffer = NULL;

ble_uuid128_t *hk_uuids_get(uint8_t id)
{
    ble_uuid128_t *result_uuid = NULL;

    hk_ll_foreach(hk_uuids_uuids, uuid)
    {
        if (uuid->value[12] == id)
        {
            result_uuid = uuid;
            //todo break;
        }
    }

    if (result_uuid == NULL)
    {
        const uint8_t buffer[16] = {0x91, 0x52, 0x76, 0xbb, 0x26, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00,
                                    id, 0x00, 0x00, 0x00};
        hk_uuids_uuids = hk_ll_new(hk_uuids_uuids);
        ble_uuid_init_from_buf((ble_uuid_any_t *)hk_uuids_uuids, (const void *)buffer, 16);
        result_uuid = hk_uuids_uuids;
        // HK_LOGD("Service uuid not found for %d, generated new one at: %x", id, (uint)result_uuid);
    }

    return result_uuid;
}

bool hk_uuids_cmp(const ble_uuid128_t *uuid1, const ble_uuid128_t *uuid2)
{
    return ble_uuid_cmp(&uuid1->u, &uuid2->u) == 0;
}

char *hk_uuids_to_str(const ble_uuid128_t *uuid)
{
    if (hk_uuids_log_buffer == NULL)
    {
        hk_uuids_log_buffer = (char *)malloc(37);
    }
    
    ble_uuid_to_str(&uuid->u, hk_uuids_log_buffer);
    return hk_uuids_log_buffer;
}