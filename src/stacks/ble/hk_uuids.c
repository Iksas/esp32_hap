#include "hk_uuids.h"

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"
#include "../../include/hk_chrs.h"

ble_uuid128_t *hk_uuids_uuids;

const ble_uuid128_t *hk_uuids_get(uint8_t id)
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
        hk_uuids_uuids = hk_ll_init(hk_uuids_uuids);
        ble_uuid_init_from_buf((ble_uuid_any_t*)hk_uuids_uuids, (const void *)buffer, 16);
        result_uuid = hk_uuids_uuids;
        // HK_LOGD("Service uuid not found for %d, generated new one at: %x", id, (uint)result_uuid);
    }

    return result_uuid;
}

bool hk_uuids_cmp(const ble_uuid128_t *uuid1, const ble_uuid128_t *uuid2)
{
    return ble_uuid_cmp(BLE_UUID(uuid1), BLE_UUID(uuid2)) == 0;
}

void hk_uuids_to_name(const ble_uuid128_t *uuid, char str[40])
{
    if (hk_uuids_cmp(&hk_uuids_descriptor_instance_id, uuid))
    {
        sprintf(str, "InstanceId");
    }
    else
    {
        switch (uuid->value[12])
        { //todo: move to hk_chrs
        case HK_CHR_IDENTIFY:
            sprintf(str, "IDENTIFY");
            break;
        case HK_CHR_MANUFACTURER:
            sprintf(str, "MANUFACTURER");
            break;
        case HK_CHR_MODEL:
            sprintf(str, "MODEL");
            break;
        case HK_CHR_MOTION_DETECTED:
            sprintf(str, "MOTION_DETECTED");
            break;
        case HK_CHR_NAME:
            sprintf(str, "NAME");
            break;
        case HK_CHR_ON:
            sprintf(str, "ON");
            break;
        case HK_CHR_SERIAL_NUMBER:
            sprintf(str, "SERIAL_NUMBER");
            break;
        case HK_CHR_VERSION:
            sprintf(str, "VERSION");
            break;
        case HK_CHR_PAIR_SETUP:
            sprintf(str, "PAIR_SETUP");
            break;
        case HK_CHR_PAIR_VERIFY:
            sprintf(str, "PAIR_VERIFY");
            break;
        case HK_CHR_PAIRING_FEATURES:
            sprintf(str, "PAIRING_FEATURES");
            break;
        case HK_CHR_PAIRING_PAIRINGS:
            sprintf(str, "PAIRING_PAIRINGS");
            break;
        case HK_CHR_FIRMWARE_REVISION:
            sprintf(str, "FIRMWARE_REVISION");
            break;
        case HK_CHR_SERVICE_SIGNATURE:
            sprintf(str, "SERVICE_SIGNATURE");
            break;
        default:
            sprintf(str, "Unknown");
            break;
        }
    }
}