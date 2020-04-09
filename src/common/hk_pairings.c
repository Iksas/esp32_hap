#include "hk_pair_verify.h"

#include "../utils/hk_logging.h"
#include "../utils/hk_tlv.h"
#include "../utils/hk_store.h"
#include "hk_pairings_store.h"
#include "hk_pair_tlvs.h"
#include "../stacks/hk_advertising.h"

esp_err_t hk_pairings_remove(hk_tlv_t *tlv_data, hk_mem *result)
{
    HK_LOGI("Remove pairing.");

    hk_tlv_t *tlv_return = NULL;
    tlv_return = hk_tlv_add_uint8(tlv_return, HK_PAIR_TLV_STATE, HK_PAIR_TLV_STATE_M2); //state M2 is always returned

    hk_mem *device_id = hk_mem_init();
    esp_err_t ret = hk_tlv_get_mem_by_type(tlv_data, HK_PAIR_TLV_IDENTIFIER, device_id);
    if (!ret)
    {
        bool is_admin;
        hk_pairings_store_is_admin(device_id, &is_admin);
        if (is_admin)
        {
            hk_pairings_store_remove(device_id);
            bool has_admin_pairing = false;
            hk_pairings_store_has_admin_pairing(&has_admin_pairing);
            if (!has_admin_pairing)
            {
                HK_LOGD("Removing all pairings, because no further admin pairing.");
                hk_pairings_store_remove_all();
                hk_advertising_update_paired();
            }
        }
        else
        {
            tlv_return = hk_tlv_add_uint8(tlv_return, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_AUTHENTICATION);
        }
    }
    else
    {
        tlv_return = hk_tlv_add_uint8(tlv_return, HK_PAIR_TLV_ERROR, HK_PAIR_TLV_ERROR_UNKNOWN);
    }

    hk_tlv_serialize(tlv_return, result);

    hk_tlv_free(tlv_return);
    hk_mem_free(device_id);

    return ret;
}

int hk_pairings(hk_mem *request, hk_mem *data, bool *kill_session)
{
    HK_LOGD("Pairings");
    int res = HK_RES_OK;
    hk_tlv_t *tlv_data = hk_tlv_deserialize(request);
    hk_tlv_t *type_tlv = hk_tlv_get_tlv_by_type(tlv_data, HK_PAIR_TLV_STATE);

    if (type_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        res = HK_RES_MALFORMED_REQUEST;
    }
    else if (*type_tlv->value != 1)
    {
        HK_LOGE("Unexpected state.");
        res = HK_RES_MALFORMED_REQUEST;
    }

    hk_tlv_t *method_tlv = NULL;
    if (res == HK_RES_OK)
    {
        method_tlv = hk_tlv_get_tlv_by_type(tlv_data, HK_PAIR_TLV_METHOD);
        if (method_tlv == NULL)
        {
            HK_LOGE("Could not find tlv with type method.");
            res = HK_RES_MALFORMED_REQUEST;
        }
    }

    if (res == HK_RES_OK)
    {
        switch (*method_tlv->value)
        {
        case 3:
            HK_LOGE("Adding a second device is not implemented at the moment.");
            break;
        case 4:
            hk_pairings_remove(tlv_data, data);
            break;
        case 5:
            HK_LOGE("Listing devices is not implemented at the moment.");
            break;
        default:
            HK_LOGE("Unexpected value in tlv in pairing: %d", *type_tlv->value);
            res = HK_RES_MALFORMED_REQUEST;
        }
    }

    if (res == HK_RES_OK && *method_tlv->value == 4)
    {
        *kill_session = true; // We kill all sessions, anyway if device was removed. This is for simplicity. Homekit reconnects in this case.
    }

    hk_tlv_free(tlv_data);

    return res;
}