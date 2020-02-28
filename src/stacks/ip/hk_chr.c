#include "hk_chrs.h"

#include "../../common/hk_chrs_properties.h"
#include "../../include/homekit_srvs.h"
#include "../../include/homekit_chrs.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_res.h"
#include "../../utils/hk_ll.h"
#include "hk_html.h"
#include "hk_html_parser.h"
#include "hk_accessories_serializer.h"
#include "hk_subscription_store.h"

#include <cJSON.h>
#include <stdbool.h>

char *hk_chrs_get_next_id_pair(char *ids, int *result)
{
    sscanf(ids, "%d.%d", &result[0], &result[1]);
    char *next_ids = strchr(ids, ',');

    if (next_ids == NULL)
    {
        return NULL;
    }

    return next_ids + 1;
}

void hk_chrs_get(hk_session_t *session)
{
    hk_mem *ids = hk_mem_create();
    session->response->content_type = HK_SESSION_CONTENT_JSON;
    esp_err_t res = hk_html_parser_get_query_value(session->request->query, "id", ids);

    int results[2];
    cJSON *j_root = cJSON_CreateObject();
    if (res == ESP_OK)
    {
        cJSON *j_chrs = cJSON_CreateArray();
        cJSON_AddItemToObject(j_root, "chrs", j_chrs);
        char *id_ptr = ids->ptr;
        session->response->result = HK_RES_OK;
        while (id_ptr != NULL)
        {
            id_ptr = hk_chrs_get_next_id_pair(id_ptr, results);
            hk_chr_t *chr = hk_accessories_store_get_chr(results[0], results[1]);
            if (chr == NULL)
            {
                HK_LOGE("Could not find chr %d.%d.", results[0], results[1]);
                session->response->result = HK_RES_UNKNOWN;
                break;
            }

            cJSON *j_chr = cJSON_CreateObject();
            cJSON_AddNumberToObject(j_chr, "aid", results[0]);
            cJSON_AddNumberToObject(j_chr, "iid", results[1]);
            hk_accessories_serializer_value(chr, j_chr);
            cJSON_AddItemToArray(j_chrs, j_chr);
        }
    }

    if (session->response->result == HK_RES_OK)
    {
        char *serialized = cJSON_PrintUnformatted(j_root);
        hk_mem_append_string(session->response->content, (const char *)serialized);
        free(serialized);
    }

    HK_LOGD("%d - Returning value for chr %d.%d.", session->socket, results[0], results[1]);
    hk_session_send(session);
    hk_mem_free(ids);
    cJSON_Delete(j_root);
}

void hk_chrs_notify(void *chr_ptr)
{
    if (!chr_ptr)
    {
        HK_LOGE("No chr was given, to notify.");
        return;
    }

    hk_chr_t *chr = (hk_chr_t *)chr_ptr;
    hk_session_t **session_list = hk_subscription_store_get_sessions(chr);

    if (session_list == NULL)
    {
        HK_LOGD("Cant notify, because nothing is subscribed for chr %d.%d.", chr->aid, chr->iid);
        return;
    }

    const double aid = chr->aid;
    const double iid = chr->iid;

    cJSON *j_chr = cJSON_CreateObject();
    cJSON_AddNumberToObject(j_chr, "aid", aid);
    cJSON_AddNumberToObject(j_chr, "iid", iid);
    cJSON_AddNumberToObject(j_chr, "status", 0); // todo: is this needed?
    hk_accessories_serializer_value(chr, j_chr);

    cJSON *j_chrs = cJSON_CreateArray();
    cJSON_AddItemToArray(j_chrs, j_chr);

    cJSON *j_root = cJSON_CreateObject();
    cJSON_AddItemToObject(j_root, "chrs", j_chrs);

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

void hk_chrs_write(hk_session_t *session, cJSON *j_chr)
{
    esp_err_t ret = ESP_OK;

    cJSON *j_aid = cJSON_GetObjectItem(j_chr, "aid");
    int aid = j_aid->valueint;

    cJSON *j_iid = cJSON_GetObjectItem(j_chr, "iid");
    int iid = j_iid->valueint;

    hk_chr_t *chr = hk_accessories_store_get_chr(aid, iid);
    if (chr == NULL)
    {
        HK_LOGE("Could not find chr %d.%d.", aid, iid);
        ret = ESP_FAIL;
    }

    if (!chr->write)
    {
        HK_LOGE("Could not write chr %d.%d. It has no write function.", aid, iid);
        ret = ESP_FAIL;
    }

    if (!ret)
    {
        cJSON *j_value = cJSON_GetObjectItem(j_chr, "value");
        hk_format_t format = hk_chrs_properties_get_type(chr->type);
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
            HK_LOGD("%d - Writing chr %d.%d.", session->socket, aid, iid);
            chr->write(value);
            hk_html_response_send_empty(session, HK_HTML_204);
            hk_chrs_notify(chr);
        }
    }

    if (ret)
    {
        hk_html_response_send_empty(session, HK_HTML_500);
    }
}

void hk_chr_subscribe(hk_session_t *session, cJSON *j_chr, cJSON *j_root)
{
    cJSON *j_aid = cJSON_GetObjectItem(j_chr, "aid");
    int aid = j_aid->valueint;

    cJSON *j_iid = cJSON_GetObjectItem(j_chr, "iid");
    int iid = j_iid->valueint;

    HK_LOGD("%d - Subscription request for chr %d.%d.", session->socket, aid, iid);
    hk_chr_t *chr = hk_accessories_store_get_chr(aid, iid);
    if (chr == NULL)
    {
        HK_LOGE("Could not find chr %d.%d.", aid, iid);
        return;
    }

    hk_subscription_store_add_session(chr, session);

    hk_html_response_send_empty(session, HK_HTML_204);

    hk_chrs_notify(chr);
}

void hk_chr_unsubscribe(hk_session_t *session, cJSON *j_chr)
{
    cJSON *j_aid = cJSON_GetObjectItem(j_chr, "aid");
    int aid = j_aid->valueint;

    cJSON *j_iid = cJSON_GetObjectItem(j_chr, "iid");
    int iid = j_iid->valueint;

    hk_chr_t *chr = hk_accessories_store_get_chr(aid, iid);
    HK_LOGD("%d - Request for removing subscription for chr %d.%d (%x).", 
        session->socket, aid, iid, (unsigned int)chr);
    if (chr == NULL)
    {
        HK_LOGE("Could not find chr %d.%d.", aid, iid);
        return;
    }

    hk_subscription_store_remove_session_from_subscription(chr, session);

    hk_html_response_send_empty(session, HK_HTML_204);
}

void hk_chrs_put(hk_session_t *session)
{
    session->response->content_type = HK_SESSION_CONTENT_JSON;
    
    cJSON *j_root = cJSON_Parse((const char *)session->request->content->ptr);
    if (j_root == NULL)
    {
        HK_LOGE("Failed to parse request for chrs put: %s", session->request->content->ptr);
        session->response->result = HK_RES_UNKNOWN;
    }

    if (session->response->result == HK_RES_OK)
    {
        cJSON *j_chrs = cJSON_GetObjectItem(j_root, "chrs");
        for (int i = 0; i < cJSON_GetArraySize(j_chrs) && session->response->result == HK_RES_OK; i++)
        {
            cJSON *j_chr = cJSON_GetArrayItem(j_chrs, i);
            if (j_chr == NULL)
            {
                HK_LOGE("Could not find first element in chrs put.");
                session->response->result = HK_RES_UNKNOWN;
                break;
            }

            if (cJSON_HasObjectItem(j_chr, "ev"))
            {
                cJSON *j_ev = cJSON_GetObjectItem(j_chr, "ev");
                if (cJSON_IsTrue(j_ev))
                {
                    hk_chr_subscribe(session, j_chr, j_root);
                }
                else
                {
                    hk_chr_unsubscribe(session, j_chr);
                }
            }
            else
            {
                hk_chrs_write(session, j_chr);
            }
        }
    }

    cJSON_Delete(j_root);
}

void hk_chrs_identify(hk_session_t *session)
{
    hk_chr_t *chr = hk_accessories_store_get_identify_chr();
    if (chr == NULL)
    {
        HK_LOGE("Could not find identify chr.");
        session->response->result = HK_RES_UNKNOWN;
    }

    if (session->response->result == HK_RES_OK)
    {
        HK_LOGE("%d - Calling write on identify chr!", session->socket);
        chr->write(NULL);
    }

    hk_session_send(session);
}