#include "hk_store.h"

#include <esp_system.h>

#include "hk_logging.h"
#include "hk_util.h"

nvs_handle hk_store_handle;
const char *hk_store_name = "hk_store";

#define RUN_AND_CHECK_STORE(ret, func, args...)                          \
    ret = func(args);                                                    \
    if (ret == ESP_ERR_NVS_NOT_FOUND)                                    \
    {                                                                    \
        ret = ESP_ERR_NOT_FOUND;                                         \
    }                                                                    \
    else if (ret == ESP_ERR_NVS_KEY_TOO_LONG)                            \
    {                                                                    \
        HK_LOGE("Error executing: ESP_ERR_NVS_KEY_TOO_LONG (4361). The maximum allowed key length is 15."); \
    }                                                                    \
    else if (ret)                                                        \
    {                                                                    \
        HK_LOGE("Error executing: %s (%d)", esp_err_to_name(ret), ret); \
    }

// esp_err_t hk_store_bool_get(const char *key, bool *value)
// {
//     esp_err_t ret = ESP_OK;
//     RUN_AND_CHECK_STORE(ret, nvs_get_u8, hk_store_handle, key, (uint8_t *)value);
//     return ret;
// }

// esp_err_t hk_store_bool_set(const char *key, bool value)
// {
//     esp_err_t ret = ESP_OK;
//     RUN_AND_CHECK_STORE(ret, nvs_set_u8, hk_store_handle, key, value);
//     return ret;
// }

// todo: static
esp_err_t hk_store_u8_get(const char *key, uint8_t *value)
{
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK_STORE(ret, nvs_get_u8, hk_store_handle, key, value);
    return ret;
}

esp_err_t hk_store_u8_set(const char *key, uint8_t value)
{
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK_STORE(ret, nvs_set_u8, hk_store_handle, key, value);
    return ret;
}

esp_err_t hk_store_u16_get(const char *key, uint16_t *value)
{
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK_STORE(ret, nvs_get_u16, hk_store_handle, key, value);
    return ret;
}

esp_err_t hk_store_u16_set(const char *key, uint16_t value)
{
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK_STORE(ret, nvs_set_u16, hk_store_handle, key, value);
    return ret;
}

esp_err_t hk_store_blob_get(const char *key, hk_mem *value)
{
    size_t required_size = 0;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK_STORE(ret, nvs_get_blob, hk_store_handle, key, NULL, &required_size);
    hk_mem_set(value, required_size);
    RUN_AND_CHECK_STORE(ret, nvs_get_blob, hk_store_handle, key, value->ptr, &required_size);

    return ret;
}

esp_err_t hk_store_blob_set(const char *key, hk_mem *value)
{
    return nvs_set_blob(hk_store_handle, key, value->ptr, value->size);
}

esp_err_t hk_store_erase(const char *key)
{
    esp_err_t ret = nvs_erase_key(hk_store_handle, key);
    if (ret == ESP_ERR_NVS_NOT_FOUND)
    {
        ret = ESP_OK;
    }

    return ret;
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