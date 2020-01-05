#include "hk_characteristics.h"

#include "../../common/hk_accessories_store.h"
#include "../../include/homekit_services.h"
#include "../../include/homekit_characteristics.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_res.h"
#include "../../utils/hk_ll.h"
#include "hk_html.h"
#include "hk_html_parser.h"
#include "hk_accessories_serializer.h"
#include "hk_subscription_store.h"

#include <cJSON.h>
#include <stdbool.h>

char *hk_characteristics_get_next_id_pair(char *ids, int *result)
{
    sscanf(ids, "%d.%d", &result[0], &result[1]);
    char *next_ids = strchr(ids, ',');

    if (next_ids == NULL)
    {
        return NULL;
    }

    return next_ids + 1;
}

void hk_characteristics_get(hk_session_t *session)
{
    hk_mem *ids = hk_mem_create();
    session->response->content_type = HK_SESSION_CONTENT_JSON;
    esp_err_t res = hk_html_parser_get_query_value(session->request->query, "id", ids);

    int results[2];
    cJSON *j_root = cJSON_CreateObject();
    if (res == ESP_OK)
    {
        cJSON *j_characteristics = cJSON_CreateArray();
        cJSON_AddItemToObject(j_root, "characteristics", j_characteristics);
        char *id_ptr = ids->ptr;
        session->response->result = HK_RES_OK;
        while (id_ptr != NULL)
        {
            id_ptr = hk_characteristics_get_next_id_pair(id_ptr, results);
            hk_characteristic_t *characteristic = hk_accessories_store_get_characteristic(results[0], results[1]);
            if (characteristic == NULL)
            {
                HK_LOGE("Could not find characteristic %d.%d.", results[0], results[1]);
                session->response->result = HK_RES_UNKNOWN;
                break;
            }

            cJSON *j_characteristic = cJSON_CreateObject();
            cJSON_AddNumberToObject(j_characteristic, "aid", results[0]);
            cJSON_AddNumberToObject(j_characteristic, "iid", results[1]);
            hk_accessories_serializer_value(characteristic, j_characteristic);
            cJSON_AddItemToArray(j_characteristics, j_characteristic);
        }
    }

    if (session->response->result == HK_RES_OK)
    {
        char *serialized = cJSON_PrintUnformatted(j_root);
        hk_mem_append_string(session->response->content, (const char *)serialized);
        free(serialized);
    }

    HK_LOGD("%d - Returning value for characteristic %d.%d.", session->socket, results[0], results[1]);
    hk_session_send(session);
    hk_mem_free(ids);
    cJSON_Delete(j_root);
}

void hk_characteristics_notify(void *characteristic_ptr)
{
    if (!characteristic_ptr)
    {
        HK_LOGE("No characteristic was given, to notify.");
        return;
    }

    hk_characteristic_t *characteristic = (hk_characteristic_t *)characteristic_ptr;
    hk_session_t **session_list = hk_subscription_store_get_sessions(characteristic);

    if (session_list == NULL)
    {
        HK_LOGD("Cant notify, because nothing is subscribed for characteristic %d.%d.", characteristic->aid, characteristic->iid);
        return;
    }

    const double aid = characteristic->aid;
    const double iid = characteristic->iid;

    cJSON *j_characteristic = cJSON_CreateObject();
    cJSON_AddNumberToObject(j_characteristic, "aid", aid);
    cJSON_AddNumberToObject(j_characteristic, "iid", iid);
    cJSON_AddNumberToObject(j_characteristic, "status", 0); //todo: is this needed?
    hk_accessories_serializer_value(characteristic, j_characteristic);

    cJSON *j_characteristics = cJSON_CreateArray();
    cJSON_AddItemToArray(j_characteristics, j_characteristic);

    cJSON *j_root = cJSON_CreateObject();
    cJSON_AddItemToObject(j_root, "characteristics", j_characteristics);

    char *serialized = cJSON_PrintUnformatted(j_root);
    cJSON_Delete(j_root);

    hk_ll_foreach(session_list, current_session)
    {
        hk_session_t *session = *current_session;
        hk_mem_append_string(session->response->content, (const char *)serialized);
        hk_html_append_response_start(session, HK_HTML_PROT_EVENT, HK_HTML_200);
        hk_html_append_header(session, "Content-Type", HK_HTML_CONTENT_JSON);
        HK_LOGD("%d - Sending change notification: %s", session->socket, serialized);
        hk_html_response_send(session);
    }

    free(serialized);
}

void hk_characteristics_write(hk_session_t *session, cJSON *j_characteristic)
{
    esp_err_t ret = ESP_OK;

    cJSON *j_aid = cJSON_GetObjectItem(j_characteristic, "aid");
    int aid = j_aid->valueint;

    cJSON *j_iid = cJSON_GetObjectItem(j_characteristic, "iid");
    int iid = j_iid->valueint;

    hk_characteristic_t *characteristic = hk_accessories_store_get_characteristic(aid, iid);
    if (characteristic == NULL)
    {
        HK_LOGE("Could not find characteristic %d.%d.", aid, iid);
        ret = ESP_FAIL;
    }

    if (!characteristic->write)
    {
        HK_LOGE("Could not write characteristic %d.%d. It has no write function.", aid, iid);
        ret = ESP_FAIL;
    }

    if (!ret)
    {
        cJSON *j_value = cJSON_GetObjectItem(j_characteristic, "value");
        hk_format_t format = hk_accessories_store_get_format(characteristic->type);
        void *value = NULL;
        bool bool_value = false;

        switch (format)
        {
        case HK_FORMAT_BOOL:
            if (j_value->type == cJSON_True)
            {
                bool_value = true;
            }
            else if (j_value->type == cJSON_False)
            {
                bool_value = false;
            }
            else if (j_value->type == cJSON_Number && (j_value->valueint == 0 || j_value->valueint == 1))
            {
                bool_value = j_value->valueint == 1;
            }
            else
            {
                HK_LOGE("Failed to update %d.%d: value is not a boolean or 0/1", aid, iid);
                ret = ESP_FAIL;
            }
            value = &bool_value;
            break;
        case HK_FORMAT_UINT8:
        case HK_FORMAT_UINT16:
        case HK_FORMAT_UINT32:
        case HK_FORMAT_UINT64:
        case HK_FORMAT_INT:
            // We accept boolean values here in order to fix a bug in HomeKit. HomeKit sometimes sends a boolean
            // instead of an integer of value 0 or 1.
            if (j_value->type != cJSON_Number && j_value->type != cJSON_False && j_value->type != cJSON_True)
            {
                HK_LOGE("Failed to update %d.%d: value is not a number", aid, iid);
                ret = ESP_FAIL;
            }

            value = &j_value->valueint;

            break;
        case HK_FORMAT_FLOAT:
            if (j_value->type != cJSON_Number)
            {
                HK_LOGE("Failed to update %d.%d: value is not a number", aid, iid);
                ret = ESP_FAIL;
            }

            value = &j_value->valuedouble;

            break;
        case HK_FORMAT_STRING:
            if (j_value->type != cJSON_String)
            {
                HK_LOGE("Failed to update %d.%d: value is not a string", aid, iid);
                ret = ESP_FAIL;
            }

            value = j_value->valuestring;
            break;
        case HK_FORMAT_TLV8:
            HK_LOGW("Writing tlv not implemented.");
            break;
        case HK_FORMAT_DATA:
            HK_LOGW("Writing data not implemented.");
            break;
        case HK_FORMAT_UNKNOWN:
            HK_LOGE("Error: unknown format.");
            break;
        }

        if (!ret)
        {
            HK_LOGD("%d - Writing characteristic %d.%d.", session->socket, aid, iid);
            characteristic->write(value);
            hk_html_response_send_empty(session, HK_HTML_204);
            hk_characteristics_notify(characteristic);
        }
    }

    if (ret)
    {
        hk_html_response_send_empty(session, HK_HTML_500);
    }
}

void hk_characteristic_subscribe(hk_session_t *session, cJSON *j_characteristic, cJSON *j_root)
{
    cJSON *j_aid = cJSON_GetObjectItem(j_characteristic, "aid");
    int aid = j_aid->valueint;

    cJSON *j_iid = cJSON_GetObjectItem(j_characteristic, "iid");
    int iid = j_iid->valueint;

    HK_LOGD("%d - Subscription request for characteristic %d.%d.", session->socket, aid, iid);
    hk_characteristic_t *characteristic = hk_accessories_store_get_characteristic(aid, iid);
    if (characteristic == NULL)
    {
        HK_LOGE("Could not find characteristic %d.%d.", aid, iid);
        return;
    }

    hk_subscription_store_add_session(characteristic, session);

    hk_html_response_send_empty(session, HK_HTML_204);

    hk_characteristics_notify(characteristic);
}

void hk_characteristic_unsubscribe(hk_session_t *session, cJSON *j_characteristic)
{
    cJSON *j_aid = cJSON_GetObjectItem(j_characteristic, "aid");
    int aid = j_aid->valueint;

    cJSON *j_iid = cJSON_GetObjectItem(j_characteristic, "iid");
    int iid = j_iid->valueint;

    hk_characteristic_t *characteristic = hk_accessories_store_get_characteristic(aid, iid);
    HK_LOGD("%d - Request for removing subscription for characteristic %d.%d (%x).", 
        session->socket, aid, iid, (unsigned int)characteristic);
    if (characteristic == NULL)
    {
        HK_LOGE("Could not find characteristic %d.%d.", aid, iid);
        return;
    }

    hk_subscription_store_remove_session_from_subscription(characteristic, session);

    hk_html_response_send_empty(session, HK_HTML_204);
}

void hk_characteristics_put(hk_session_t *session)
{
    session->response->content_type = HK_SESSION_CONTENT_JSON;
    
    cJSON *j_root = cJSON_Parse((const char *)session->request->content->ptr);
    if (j_root == NULL)
    {
        HK_LOGE("Failed to parse request for characteristics put: %s", session->request->content->ptr);
        session->response->result = HK_RES_UNKNOWN;
    }

    if (session->response->result == HK_RES_OK)
    {
        cJSON *j_characteristics = cJSON_GetObjectItem(j_root, "characteristics");
        for (int i = 0; i < cJSON_GetArraySize(j_characteristics) && session->response->result == HK_RES_OK; i++)
        {
            cJSON *j_characteristic = cJSON_GetArrayItem(j_characteristics, i);
            if (j_characteristic == NULL)
            {
                HK_LOGE("Could not find first element in characteristics put.");
                session->response->result = HK_RES_UNKNOWN;
                break;
            }

            if (cJSON_HasObjectItem(j_characteristic, "ev"))
            {
                cJSON *j_ev = cJSON_GetObjectItem(j_characteristic, "ev");
                if (cJSON_IsTrue(j_ev))
                {
                    hk_characteristic_subscribe(session, j_characteristic, j_root);
                }
                else
                {
                    hk_characteristic_unsubscribe(session, j_characteristic);
                }
            }
            else
            {
                hk_characteristics_write(session, j_characteristic);
            }
        }
    }

    cJSON_Delete(j_root);
}

void hk_characteristics_identify(hk_session_t *session)
{
    hk_characteristic_t *characteristic = hk_accessories_store_get_identify_characteristic();
    if (characteristic == NULL)
    {
        HK_LOGE("Could not find identify characteristic.");
        session->response->result = HK_RES_UNKNOWN;
    }

    if (session->response->result == HK_RES_OK)
    {
        HK_LOGE("%d - Calling write on identify characteristic!", session->socket);
        characteristic->write(NULL);
    }

    hk_session_send(session);
}