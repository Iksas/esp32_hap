#include "hk_store.h"
#include "hk_logging.h"

#include <esp_system.h>
#include <nvs_flash.h>

#define HK_STORE_PAIRED "hk_paired"
#define HK_STORE_ACC_PRV_KEY "hk_acc_prv_key"
#define HK_STORE_ACC_PUB_KEY "hk_acc_pub_key"
#define HK_STORE_PAIRINGS "hk_pairings"
#define HK_STORE_CONFIGURATION "hk_config"

nvs_handle hk_store_handle;
const char *hk_store_name = "hk_store";
const char *hk_store_code;

static void hk_store_uint8_t_set(const char *key, uint8_t value)
{
    ESP_ERROR_CHECK(nvs_set_u8(hk_store_handle, key, value));
}

static esp_err_t hk_store_uint8_t_get(const char *key, uint8_t *value)
{
    esp_err_t ret = nvs_get_u8(hk_store_handle, key, value);
    if (ret != ESP_ERR_NVS_NOT_FOUND)
    {
        ESP_ERROR_CHECK(ret);
    }

    return ret;
}

static void hk_store_blob_get(const char *key, hk_mem *value)
{
    size_t required_size;
    ESP_ERROR_CHECK(nvs_get_blob(hk_store_handle, key, NULL, &required_size));

    hk_mem_set(value, required_size);
    ESP_ERROR_CHECK(nvs_get_blob(hk_store_handle, key, value->ptr, &required_size));
}

static void hk_store_blob_set(const char *key, hk_mem *value)
{
    ESP_ERROR_CHECK(nvs_set_blob(hk_store_handle, key, value->ptr, value->size));
}

bool hk_store_keys_can_get()
{
    size_t required_size;
    bool can_get_priv_key = nvs_get_blob(hk_store_handle, HK_STORE_ACC_PRV_KEY, NULL, &required_size) == ESP_OK;
    bool can_get_pub_key = nvs_get_blob(hk_store_handle, HK_STORE_ACC_PUB_KEY, NULL, &required_size) == ESP_OK;
    return can_get_priv_key && can_get_pub_key;
}

void hk_store_key_priv_get(hk_mem *value)
{
    hk_store_blob_get(HK_STORE_ACC_PRV_KEY, value);
}

void hk_store_key_priv_set(hk_mem *value)
{
    hk_store_blob_set(HK_STORE_ACC_PRV_KEY, value);
}

void hk_store_key_pub_get(hk_mem *value)
{
    hk_store_blob_get(HK_STORE_ACC_PUB_KEY, value);
}

void hk_store_key_pub_set(hk_mem *value)
{
    hk_store_blob_set(HK_STORE_ACC_PUB_KEY, value);
}

void hk_store_pairings_remove()
{
    size_t required_size = -1;
    esp_err_t ret = nvs_get_blob(hk_store_handle, HK_STORE_PAIRINGS, NULL, &required_size);
    if (ret == ESP_OK && required_size > 0)
    {
        ESP_ERROR_CHECK(nvs_erase_key(hk_store_handle, HK_STORE_PAIRINGS));
    }
}

void hk_store_pairings_get(hk_mem *pairings)
{
    size_t required_size;
    if (nvs_get_blob(hk_store_handle, HK_STORE_PAIRINGS, NULL, &required_size) == ESP_OK)
    {
        hk_mem_set(pairings, required_size);
        ESP_ERROR_CHECK(nvs_get_blob(hk_store_handle, HK_STORE_PAIRINGS, pairings->ptr, &required_size));
    }
}

void hk_store_pairings_set(hk_mem *pairings)
{
    hk_store_blob_set(HK_STORE_PAIRINGS, pairings);
}

const char *hk_store_code_get()
{
    return hk_store_code;
}

void hk_store_code_set(const char *code)
{
    hk_store_code = code;
}

uint8_t hk_store_configuration_get()
{
    uint8_t value;
    hk_store_uint8_t_get(HK_STORE_CONFIGURATION, &value);
    return value;
}

void hk_store_configuration_set(uint8_t configuration)
{
    hk_store_uint8_t_set(HK_STORE_CONFIGURATION, configuration);
}

esp_err_t hk_store_init()
{
    HK_LOGD("Initializing key value store.");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        HK_LOGW("Error initializing hk store. Now erasing flash and trying again.");
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        err = nvs_flash_erase();
        if (err)
        {
            HK_LOGEE(err);
            return err;
        }
        err = nvs_flash_init();
        if (err)
        {
            HK_LOGEE(err);
            return err;
        }
    }
    else if (err)
    {
        HK_LOGEE(err);
    }

    if (!err)
    {
        err = nvs_open(hk_store_name, NVS_READWRITE, &hk_store_handle);
    }
    else
    {
        HK_LOGEE(err);
    }

    return err;
}

void hk_store_free()
{
    nvs_close(hk_store_handle);
}