#include "hk_pairings_store.h"

#include "../utils/hk_store.h"
#include "../utils/hk_res.h"
#include "../utils/hk_logging.h"

#include <cJSON.h>

#define HK_PAIRINGS_STORE_PAIRINGS "pairings"
#define HK_PAIRINGS_STORE_DEVICE_ID "device_id"
#define HK_PAIRINGS_STORE_DEVICE_LTPK "device_ltpk"
#define HK_PAIRINGS_STORE_IS_ADMIN "is_admin"

cJSON *hk_pairings_store_get()
{
    hk_mem *pairings = hk_mem_create();
    hk_store_pairings_get(pairings);
    cJSON *j_root = cJSON_Parse(pairings->ptr);

    hk_mem_free(pairings);

    if (j_root == NULL)
    {
        HK_LOGE("Could not deserialize pairings.");
    }

    return j_root;
}

void hk_pairings_store_set(cJSON *j_root)
{
    char *serialized = cJSON_PrintUnformatted(j_root);
    hk_mem *pairings = hk_mem_create();
    hk_mem_append_string(pairings, (const char *)serialized);
    hk_store_pairings_set(pairings);

    free(serialized);
    hk_mem_free(pairings);
}

cJSON *hk_pairings_store_get_j_parings_array(cJSON *j_root)
{
    cJSON *j_pairings = NULL;
    if (!cJSON_HasObjectItem(j_root, HK_PAIRINGS_STORE_PAIRINGS))
    {
        j_pairings = cJSON_CreateArray();
        cJSON_AddItemToObject(j_root, HK_PAIRINGS_STORE_PAIRINGS, j_pairings);
    }
    else
    {
        j_pairings = cJSON_GetObjectItem(j_root, HK_PAIRINGS_STORE_PAIRINGS);
    }

    if (j_pairings == NULL)
    {
        HK_LOGE("Could not get/create array of pairings.");
    }

    return j_pairings;
}

void hk_pairings_store_add(hk_mem *device_id, hk_mem *device_ltpk, bool is_admin)
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);

    cJSON *j_pairing = cJSON_CreateObject();

    char* str = hk_mem_get_str(device_id);
    HK_LOGV("Adding pairing for device '%s'.", str);
    cJSON_AddItemToObject(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID, cJSON_CreateString(str));
    free(str);

    str = hk_mem_get_str(device_ltpk);
    cJSON_AddItemToObject(j_pairing, HK_PAIRINGS_STORE_DEVICE_LTPK, cJSON_CreateString(str));
    free(str);

    cJSON_AddItemToObject(j_pairing, HK_PAIRINGS_STORE_IS_ADMIN, cJSON_CreateBool(is_admin));
    
    cJSON_AddItemToArray(j_pairings, j_pairing);

    hk_pairings_store_set(j_root);

    cJSON_Delete(j_root);
}

esp_err_t hk_pairings_store_ltpk_get(hk_mem *device_id, hk_mem *device_ltpk)
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);

    esp_err_t ret = HK_RES_UNKNOWN;

    cJSON *j_pairing = j_pairings->child;
    while (j_pairing)
    {
        if (cJSON_HasObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID))
        {
            cJSON *j_device_id = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID);

            if (memcmp(j_device_id->valuestring, device_id->ptr, device_id->size) == 0)
            {
                cJSON *j_device_ltpk = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_LTPK);
                hk_mem_append_string(device_ltpk, j_device_ltpk->valuestring);
                ret = HK_RES_OK;
                j_pairing = NULL;
            }
        }
        else
        {
            HK_LOGW("Found pairing without device id?");
        }

        if (j_pairing != NULL)
        {
            j_pairing = j_pairing->next;
        }
    }

    cJSON_Delete(j_root);

    if (ret != HK_RES_OK)
    {
        char* id_str =  hk_mem_get_str(device_id);
        HK_LOGE("A long term public key was not found for device id %s", id_str);
        free(id_str);
    }

    return ret;
}

void hk_pairings_store_remove(hk_mem *device_id)
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);
    cJSON *j_pairing = j_pairings->child;

    while (j_pairing)
    {
        if (cJSON_HasObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID))
        {
            cJSON *j_device_id = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID);
            if (memcmp(j_device_id->valuestring, device_id->ptr, device_id->size) == 0)
            {
                HK_LOGD("Deleting pairing for device '%s'.", j_device_id->valuestring);
                cJSON_DetachItemViaPointer(j_pairings, j_pairing);
                cJSON_Delete(j_pairing);
                j_pairing = NULL;
            }
        }
        else
        {
            HK_LOGW("Found pairing without device id?");
        }

        if (j_pairing != NULL)
        {
            j_pairing = j_pairing->next;
        }
    }

    hk_pairings_store_set(j_root);
    cJSON_Delete(j_root);
}

bool hk_pairings_store_is_admin(hk_mem *device_id)
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);

    bool is_admin = false;

    cJSON *j_pairing = j_pairings->child;
    while (j_pairing)
    {
        if (cJSON_HasObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID))
        {
            cJSON *j_device_id = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID);

            if (memcmp(j_device_id->valuestring, device_id->ptr, device_id->size) == 0)
            {
                cJSON *j_is_admin = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_IS_ADMIN);
                if (cJSON_IsTrue(j_is_admin))
                {
                    is_admin = true;
                    j_pairing = NULL;
                }
            }
        }
        else
        {
            HK_LOGW("Found pairing without device id?");
        }

        if (j_pairing != NULL)
        {
            j_pairing = j_pairing->next;
        }
    }

    cJSON_Delete(j_root);

    return is_admin;
}

bool hk_pairings_store_has_admin_pairing()
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);
    cJSON *j_pairing = j_pairings->child;

    bool admin_found = false;
    while (j_pairing)
    {
        cJSON *j_is_admin = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_IS_ADMIN);
        if (cJSON_IsTrue(j_is_admin))
        {
            admin_found = true;
            j_pairing = NULL;
        }
        else
        {
            j_pairing = j_pairing->next;
        }
    }

    cJSON_Delete(j_root);

    return admin_found;
}

bool hk_pairings_store_has_pairing()
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);
    cJSON *j_pairing = j_pairings->child;

    bool found = false;
    while (j_pairing)
    {
        found = true;
        break;
    }

    cJSON_Delete(j_root);

    return found;
}

void hk_pairings_store_remove_all()
{
    cJSON *j_root = cJSON_CreateObject();
    cJSON *j_pairings = cJSON_CreateArray();
    cJSON_AddItemToObject(j_root, HK_PAIRINGS_STORE_PAIRINGS, j_pairings);

    hk_pairings_store_set(j_root);
    cJSON_Delete(j_root);
}

esp_err_t hk_pairings_store_list()
{
    esp_err_t ret = ESP_OK;
    HK_LOGW("List pairings not implemented.");
    return ret;
}

void hk_pairings_log_devices()
{
    cJSON *j_root = hk_pairings_store_get();
    cJSON *j_pairings = hk_pairings_store_get_j_parings_array(j_root);
    cJSON *j_pairing = j_pairings->child;

    HK_LOGD("We are coupled with the following devices:");
    while (j_pairing)
    {
        cJSON *j_device_id = cJSON_GetObjectItem(j_pairing, HK_PAIRINGS_STORE_DEVICE_ID);
        HK_LOGD("   %s", j_device_id->valuestring);

        j_pairing = j_pairing->next;
    }

    cJSON_Delete(j_root);
}