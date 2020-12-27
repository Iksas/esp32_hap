#include "hk_chrs.h"

#include "../../common/hk_chrs_properties.h"
#include "../../include/hk_srvs.h"
#include "../../include/hk_chrs.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_ll.h"
#include "../../utils/hk_util.h"
#include "hk_server.h"
#include "hk_accessories_serializer.h"
#include "hk_subscription_store.h"
#include "hk_advertising.h"

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

esp_err_t hk_chrs_get(char *ids, hk_mem *response)
{
    esp_err_t ret = ESP_OK;

    int results[2];
    cJSON *j_root = cJSON_CreateObject();
    cJSON *j_chrs = cJSON_CreateArray();
    cJSON_AddItemToObject(j_root, "characteristics", j_chrs);

    while (ids != NULL)
    {
        ids = hk_chrs_get_next_id_pair(ids, results);
        hk_chr_t *chr = hk_accessories_store_get_chr(results[0], results[1]);
        if (chr == NULL)
        {
            HK_LOGE("Could not find chr %d.%d.", results[0], results[1]);
            ret = ESP_ERR_NOT_FOUND;
            break;
        }

        cJSON *j_chr = cJSON_CreateObject();
        cJSON_AddNumberToObject(j_chr, "aid", results[0]);
        cJSON_AddNumberToObject(j_chr, "iid", results[1]);
        hk_accessories_serializer_value(chr, j_chr);
        cJSON_AddItemToArray(j_chrs, j_chr);
    }

    if (ret == ESP_OK)
    {
        char *serialized = cJSON_PrintUnformatted(j_root);
        hk_mem_append_string(response, (const char *)serialized);
        free(serialized);
    }

    cJSON_Delete(j_root);

    return ret;
}

esp_err_t hk_chrs_notify(void *chr_ptr)
{
    esp_err_t ret = ESP_OK;
    if (!chr_ptr)
    {
        HK_LOGE("No chr was given, to notify.");
        return ESP_ERR_INVALID_ARG;
    }

    // increase global state
    hk_advertising_global_state_next();

    // fire event
    hk_chr_t *chr = (hk_chr_t *)chr_ptr;
    const uint8_t aid = chr->aid;
    const uint8_t iid = chr->iid;
    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr, &sockets, &number_of_sockets);

    if (ret == ESP_ERR_NOT_FOUND)
    {
        HK_LOGD("Cant notify, because nothing is subscribed for characteristics %d.%d.", aid, iid);
        return ret;
    }
    else if (ret != ESP_OK)
    {
        HK_LOGE("Error getting characteristics %d.%d to notify.", aid, iid);
        return ret;
    }

    if (sockets == NULL || number_of_sockets < 1)
    {
        HK_LOGD("Cant notify, because nothing is subscribed for chr %d.%d.", aid, iid);
        return ret;
    }

    cJSON *j_chr = cJSON_CreateObject();
    cJSON_AddNumberToObject(j_chr, "aid", (const double)aid);
    cJSON_AddNumberToObject(j_chr, "iid", (const double)iid);
    cJSON_AddNumberToObject(j_chr, "status", 0); // todo: is this needed?
    hk_accessories_serializer_value(chr, j_chr);

    cJSON *j_chrs = cJSON_CreateArray();
    cJSON_AddItemToArray(j_chrs, j_chr);

    cJSON *j_root = cJSON_CreateObject();
    cJSON_AddItemToObject(j_root, "characteristics", j_chrs);

    char *serialized = cJSON_PrintUnformatted(j_root);
    cJSON_Delete(j_root);

    for (size_t i = 0; i < number_of_sockets; i++)
    {
        hk_mem *message = hk_mem_init(); // is freed after being sent
        hk_mem_append_string(message, "EVENT/1.0 200 OK\n");
        hk_mem_append_string(message, "Content-Type: application/hap+json\n");
        hk_mem_append_string(message, "Content-Length: ");
        char content_length[20];
        sprintf(content_length, "%d\n\n", strlen(serialized));
        hk_mem_append_string(message, content_length);
        hk_mem_append_string(message, (const char *)serialized);
        HK_LOGD("%d - Sending change notification.", sockets[i]);
        RUN_AND_CHECK(ret, hk_server_send_async, sockets[i], message);
    }

    free(serialized);
    return ret;
}

static esp_err_t hk_chrs_write(int socket, int aid, int iid, cJSON *j_chr)
{
    esp_err_t ret = ESP_OK;
    hk_chr_t *chr = hk_accessories_store_get_chr(aid, iid);
    if (chr == NULL)
    {
        HK_LOGE("%d - Could not find chr %d.%d.", socket, aid, iid);
        ret = ESP_FAIL;
    }

    if (chr->write == NULL)
    {
        HK_LOGE("%d - Could not write chr %d.%d. It has no write function.", socket, aid, iid);
        ret = ESP_FAIL;
    }

    if (!ret)
    {
        cJSON *j_value = cJSON_GetObjectItem(j_chr, "value");
        hk_format_t format = hk_chrs_properties_get_type(chr->type);
        bool bool_value = false;
        hk_mem *write_request = hk_mem_init();

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
                HK_LOGE("%d - Failed to update %d.%d: value is not a boolean or 0/1", socket, aid, iid);
                ret = ESP_FAIL;
            }
            hk_mem_append_buffer(write_request, (char *)&bool_value, sizeof(bool));
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
                HK_LOGE("%d - Failed to update %d.%d: value is not a number", socket, aid, iid);
                ret = ESP_FAIL;
            }

            hk_mem_append_buffer(write_request, (char *)&j_value->valueint, sizeof(int));

            break;
        case HK_FORMAT_FLOAT:
            if (j_value->type != cJSON_Number)
            {
                HK_LOGE("%d - Failed to update %d.%d: value is not a number", socket, aid, iid);
                ret = ESP_FAIL;
            }

            hk_mem_append_buffer(write_request, (char *)&j_value->valuedouble, sizeof(double));

            break;
        case HK_FORMAT_STRING:
            if (j_value->type != cJSON_String)
            {
                HK_LOGE("%d - Failed to update %d.%d: value is not a string", socket, aid, iid);
                ret = ESP_FAIL;
            }

            hk_mem_append_string(write_request, j_value->valuestring);
            break;
        case HK_FORMAT_TLV8:
            HK_LOGW("%d - Writing tlv not implemented.", socket);
            break;
        case HK_FORMAT_DATA:
            HK_LOGW("%d - Writing data not implemented.", socket);
            break;
        case HK_FORMAT_UNKNOWN:
            HK_LOGE("%d - Error: unknown format.", socket);
            break;
        }

        if (!ret)
        {
            HK_LOGD("%d - Writing chr %d.%d.", socket, aid, iid);

            ret = chr->write(write_request);
            if (ret == ESP_OK)
            {
                //todo: hk_chrs_notify(chr);
            }
            else
            {
                HK_LOGE("%d - Error writing characteristic.", socket);
            }
        }

        hk_mem_free(write_request);
    }

    return ret;
}

static esp_err_t hk_chr_subscribe(int socket, int aid, int iid)
{
    esp_err_t ret = ESP_OK;
    HK_LOGV("%d - Subscription request for chr %d.%d.", socket, aid, iid);

    hk_chr_t *chr = hk_accessories_store_get_chr(aid, iid);
    if (chr == NULL)
    {
        HK_LOGE("Could not find chr %d.%d.", aid, iid);
        ret = ESP_ERR_NOT_FOUND;
    }

    RUN_AND_CHECK(ret, hk_subscription_store_add, chr, socket);

    //todo: hk_chrs_notify(chr);
    return ret;
}

static esp_err_t hk_chr_unsubscribe(int socket, int aid, int iid)
{
    esp_err_t ret = ESP_OK;
    hk_chr_t *chr = hk_accessories_store_get_chr(aid, iid);
    HK_LOGD("%d - Request for removing subscription for chr %d.%d (%x).", socket, aid, iid, (unsigned int)chr);
    if (chr == NULL)
    {
        HK_LOGE("%d - Could not find chr %d.%d.", socket, aid, iid);
        ret = ESP_ERR_NOT_FOUND;
    }

    RUN_AND_CHECK(ret, hk_subscription_store_remove, chr, socket);

    return ret;
}

esp_err_t hk_chrs_put(hk_mem *request, void *http_handle, int socket)
{
    esp_err_t ret = ESP_OK;

    char *content = hk_mem_to_string(request);
    cJSON *j_root = cJSON_Parse((const char *)content);
    if (j_root == NULL)
    {
        HK_LOGE("Failed to parse request for chrs put.");
        ret = ESP_ERR_INVALID_ARG;
    }

    if (ret == ESP_OK)
    {
        cJSON *j_chrs = cJSON_GetObjectItem(j_root, "characteristics");
        for (int i = 0; i < cJSON_GetArraySize(j_chrs) && ret == ESP_OK; i++)
        {
            cJSON *j_chr = cJSON_GetArrayItem(j_chrs, i);
            if (j_chr == NULL)
            {
                HK_LOGE("Could not find first element in chrs put.");
                ret = ESP_ERR_INVALID_ARG;
                break;
            }

            cJSON *j_aid = cJSON_GetObjectItem(j_chr, "aid");
            cJSON *j_iid = cJSON_GetObjectItem(j_chr, "iid");

            int aid = j_aid->valueint;
            int iid = j_iid->valueint;

            if (cJSON_HasObjectItem(j_chr, "ev"))
            {
                cJSON *j_ev = cJSON_GetObjectItem(j_chr, "ev");

                if (cJSON_IsTrue(j_ev))
                {
                    RUN_AND_CHECK(ret, hk_chr_subscribe, socket, aid, iid);
                }
                else
                {
                    RUN_AND_CHECK(ret, hk_chr_unsubscribe, socket, aid, iid);
                }
            }
            else
            {
                RUN_AND_CHECK(ret, hk_chrs_write, socket, aid, iid, j_chr);
            }
        }
    }

    cJSON_Delete(j_root);
    free(content);

    return ret;
}

esp_err_t hk_chrs_identify(int socket)
{
    hk_chr_t *chr = hk_accessories_store_get_identify_chr();
    if (chr == NULL)
    {
        HK_LOGE("Could not find identify chr.");
        return ESP_ERR_NOT_FOUND;
    }

    esp_err_t ret = ESP_OK;
    HK_LOGD("%d - Calling write on identify chr!", socket);
    RUN_AND_CHECK(ret, chr->write, NULL);

    return ret;
}