#include "hk_pair_verify.h"

#include "utils/hk_logging.h"
#include "utils/hk_tlv.h"
#include "utils/hk_store.h"
#include "hk_html.h"
#include "hk_pairings_store.h"
#include "hk_advertising.h"

esp_err_t hk_pairings_remove(hk_tlv_t *tlv_data, hk_mem *result)
{
    HK_LOGI("Remove pairing.");

    hk_tlv_t *tlv_return = NULL;
    tlv_return = hk_tlv_add_state(tlv_return, 2); //state M2 is always returned

    hk_mem *device_id = hk_mem_create();
    esp_err_t ret = hk_tlv_get_mem_by_type(tlv_data, HK_TLV_Identifier, device_id);
    if (!ret)
    {
        if (hk_pairings_store_is_admin(device_id))
        {
            hk_pairings_store_remove(device_id);
            if (!hk_pairings_store_has_admin_pairing())
            {
                HK_LOGD("Removing all pairings, because no further admin pairing.");
                hk_pairings_store_remove_all();
                hk_store_is_paired_set(false);
                hk_advertising_update_paired(false);
            }
        }
        else
        {
            tlv_return = hk_tlv_add_error(tlv_return, HK_TLV_ERROR_Authentication);
        }
    }
    else
    {
        tlv_return = hk_tlv_add_error(tlv_return, HK_TLV_ERROR_Unknown);
    }

    hk_tlv_serialize(tlv_return, result);

    hk_tlv_free(tlv_return);
    hk_mem_free(device_id);

    return ret;
}

void hk_pairings(hk_session_t *session)
{
    HK_LOGD("Pairings");

    hk_tlv_t *tlv_data = hk_tlv_deserialize(session->request->content);
    hk_tlv_t *type_tlv = hk_tlv_get_tlv_by_type(tlv_data, HK_TLV_State);

    if (type_tlv == NULL)
    {
        HK_LOGE("Could not find tlv with type state.");
        session->response->result = HK_RES_MALFORMED_REQUEST;
    }
    else if (*type_tlv->value != 1)
    {
        HK_LOGE("Unexpected state.");
        session->response->result = HK_RES_MALFORMED_REQUEST;
    }

    hk_tlv_t *method_tlv = NULL;
    if (session->response->result == HK_RES_OK)
    {
        method_tlv = hk_tlv_get_tlv_by_type(tlv_data, HK_TLV_Method);
        if (method_tlv == NULL)
        {
            HK_LOGE("Could not find tlv with type method.");
            session->response->result = HK_RES_MALFORMED_REQUEST;
        }
    }

    if (session->response->result == HK_RES_OK)
    {
        switch (*method_tlv->value)
        {
        case 3:
            HK_LOGE("Adding a second device is not implemented at the moment.");
            break;
        case 4:
            hk_pairings_remove(tlv_data, session->response->data);
            break;
        case 5:
            HK_LOGE("Listing devices is not implemented at the moment.");
            break;
        default:
            HK_LOGE("Unexpected value in tlv in pairing: %d", *type_tlv->value);
            session->response->result = HK_RES_MALFORMED_REQUEST;
        }
    }

    hk_session_send(session);

    if (session->response->result == HK_RES_OK && *method_tlv->value == 4)
    {
        HK_LOGD("%d - Killing", session->socket);
        session->kill = true; // We kill all sessions, anyway if device was removed. This is for simplicity. Homekit reconnects in this case.
    }

    hk_tlv_free(tlv_data);
}