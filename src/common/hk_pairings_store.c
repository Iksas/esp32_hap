#include "hk_pairings_store.h"

#include "../utils/hk_store.h"
#include "../utils/hk_util.h"
#include "../utils/hk_logging.h"

#define HK_PARING_STORE_KEY "hk_pairings"

typedef struct
{
    size_t id_length;
    size_t key_length;
    bool is_admin;
    const char *id;
    const char *key;
    size_t length;
} hk_pairing_store_pair;

static esp_err_t hk_pairings_store_get(hk_mem *data)
{
    return hk_store_blob_get(HK_PARING_STORE_KEY, data);
}

static esp_err_t hk_pairings_store_set(hk_mem *data)
{
    return hk_store_blob_set(HK_PARING_STORE_KEY, data);
}

static void hk_pairings_store_entry_add(hk_pairing_store_pair *entry, hk_mem *data)
{
    //append id size
    hk_mem_append_buffer(data, &entry->id_length, sizeof(size_t));
    //append key size
    hk_mem_append_buffer(data, &entry->key_length, sizeof(size_t));
    //append is admin value
    hk_mem_append_buffer(data, &entry->is_admin, sizeof(bool));
    //append id
    hk_mem_append_buffer(data, (void *)entry->id, entry->id_length);
    //append key
    hk_mem_append_buffer(data, (void *)entry->key, entry->key_length);
}

static void hk_pairings_store_entry_update(hk_pairing_store_pair *entry, char *data)
{
    entry->id_length = (size_t)data[0];
    entry->key_length = (size_t)data[sizeof(size_t)];
    entry->is_admin = (bool)data[sizeof(size_t) * 2];
    entry->id = data + 2 * sizeof(size_t) + sizeof(bool);
    entry->key = data + 2 * sizeof(size_t) + sizeof(bool) + entry->id_length;
    entry->length = 2 * sizeof(size_t) + sizeof(bool) + entry->id_length + entry->key_length;
}

static esp_err_t hk_pairings_store_entry_get(hk_pairing_store_pair *entry, hk_mem *data, hk_mem *device_id)
{
    esp_err_t ret = ESP_ERR_NOT_FOUND;
    char *device_id_str = strndup(device_id->ptr, device_id->size);

    if (data->size > 0)
    {
        for (size_t data_read = 0; data_read < data->size;)
        {
            hk_pairings_store_entry_update(entry, data->ptr + data_read);
            char *entry_str = strndup(entry->id, entry->id_length);
            if (strcmp(device_id_str, entry_str) == 0)
            {
                ret = ESP_OK;
                free(entry_str);
                break;
            }

            free(entry_str);

            data_read += entry->length;
        }
    }

    free(device_id_str);
    return ret;
}

esp_err_t hk_pairings_store_add(hk_mem *device_id, hk_mem *device_ltpk, bool is_admin)
{
    esp_err_t ret = ESP_OK;
    hk_mem *data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));

    hk_pairings_store_get(data);
    entry->id_length = device_id->size;
    entry->key_length = device_ltpk->size;
    entry->is_admin = is_admin;
    entry->id = device_id->ptr;
    entry->key = device_ltpk->ptr;
    hk_pairings_store_entry_add(entry, data);
    RUN_AND_CHECK(ret, hk_pairings_store_set, data);

    hk_mem_free(data);
    free(entry);

    return ret;
}

esp_err_t hk_pairings_store_device_exists(hk_mem *device_id, bool *exists)
{
    hk_mem *data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));

    hk_pairings_store_get(data);
    esp_err_t ret = hk_pairings_store_entry_get(entry, data, device_id);
    if (!ret)
    {
        *exists = true;
    }
    else if (ret == ESP_ERR_NOT_FOUND)
    {
        *exists = false;
        ret = ESP_OK;
    }

    hk_mem_free(data);
    free(entry);

    return ret;
}

esp_err_t hk_pairings_store_ltpk_get(hk_mem *device_id, hk_mem *device_ltpk)
{
    hk_mem *data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));

    hk_pairings_store_get(data);
    esp_err_t ret = hk_pairings_store_entry_get(entry, data, device_id);
    if (!ret)
    {
        hk_mem_append_buffer(device_ltpk, (void *)entry->key, entry->key_length);
    }

    hk_mem_free(data);
    free(entry);

    return ret;
}

esp_err_t hk_pairings_store_remove(hk_mem *device_id)
{
    esp_err_t ret = ESP_OK;
    hk_mem *data = hk_mem_init();
    hk_mem *new_data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));
    bool item_removed = false;
    char *device_id_str = strndup(device_id->ptr, device_id->size);

    RUN_AND_CHECK(ret, hk_pairings_store_get, data);
    if (!ret && data->size > 0)
    {
        for (size_t data_read = 0; data_read < data->size;)
        {
            hk_pairings_store_entry_update(entry, data->ptr + data_read);
            if (strcmp(device_id_str, entry->id))
            {
                item_removed = true;
            }
            else
            {
                hk_pairings_store_entry_add(entry, new_data);
            }
            data_read += entry->length;
        }
    }

    if (item_removed)
    {
        RUN_AND_CHECK(ret, hk_pairings_store_set, new_data);
    }
    else
    {
        ret = ESP_ERR_NOT_FOUND;
    }
    
    hk_mem_free(data);
    hk_mem_free(new_data);
    free(device_id_str);
    free(entry);

    return ret;
}

esp_err_t hk_pairings_store_is_admin(hk_mem *device_id, bool *is_admin)
{
    hk_mem *data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));

    *is_admin = false;
    hk_pairings_store_get(data);
    esp_err_t ret = hk_pairings_store_entry_get(entry, data, device_id);
    if (!ret)
    {
        *is_admin = entry->is_admin;
    }

    hk_mem_free(data);
    free(entry);

    return ret;
}

esp_err_t hk_pairings_store_has_admin_pairing(bool *is_admin)
{
    hk_mem *data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));
    esp_err_t ret = ESP_OK;

    *is_admin = false;
    hk_pairings_store_get(data);
    if (data->size > 0)
    {
        for (size_t data_read = 0; data_read < data->size;)
        {
            hk_pairings_store_entry_update(entry, data->ptr + data_read);
            if (entry->is_admin)
            {
                *is_admin = true;
                break;
            }
            data_read += entry->length;
        }
    }

    hk_mem_free(data);
    free(entry);

    return ret;
}

esp_err_t hk_pairings_store_has_pairing(bool *has_pairing)
{
    hk_mem *data = hk_mem_init();
    esp_err_t ret = ESP_OK;

    hk_pairings_store_get(data);
    *has_pairing = data->size > 0;

    hk_mem_free(data);

    return ret;
}

esp_err_t hk_pairings_store_remove_all()
{
    HK_LOGD("Deleting paring store.");
    esp_err_t ret = ESP_OK;
    ret = hk_store_erase(HK_PARING_STORE_KEY);
    
    if (ret == ESP_ERR_NVS_NOT_FOUND)
    {
        ret = ESP_OK;
    }
    else if(ret != ESP_OK)
    {
        HK_LOGE("Error resetting paring store: %s (%d)", esp_err_to_name(ret), ret);
    }

    return ret;
}

esp_err_t hk_pairings_log_devices()
{
    hk_mem *data = hk_mem_init();
    hk_pairing_store_pair *entry = (hk_pairing_store_pair *)malloc(sizeof(hk_pairing_store_pair));
    esp_err_t ret = ESP_OK;

    hk_pairings_store_get(data);
    if (data->size > 0)
    {
        for (size_t data_read = 0; data_read < data->size;)
        {
            hk_pairings_store_entry_update(entry, data->ptr + data_read);
            
            HK_LOGI("%s", strndup(entry->id, entry->id_length));
            data_read += entry->length;
        }
    }

    hk_mem_free(data);
    free(entry);

    return ret;
}