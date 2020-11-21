#include "hk_broadcast_key.h"

#include "../../crypto/hk_hkdf.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_global_state.h"
#include "../../common/hk_key_store.h"

#define HK_BROADCAST_KEY "hk_bk"
#define HK_BROADCAST_KEY_GLOBAL_STATE_AT_CREATION "hk_bk_gs_at_cre"

esp_err_t hk_broadcast_key_create(hk_mem *accessory_shared_secret, hk_mem *broadcast_key)
{
    esp_err_t ret = ESP_OK;
    uint16_t global_state = hk_global_state_get();
    hk_mem *accessory_public_key = hk_mem_init();

    // get public key and keys->shared_secret
    RUN_AND_CHECK(ret, hk_key_store_pub_get, accessory_public_key);

    // generate broadcast_key
    RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, accessory_shared_secret, broadcast_key, accessory_public_key, HK_HKDF_BROADCAST_ENCRYPTION_KEY_INFO);
    RUN_AND_CHECK(ret, hk_store_blob_set, HK_BROADCAST_KEY, broadcast_key);
    RUN_AND_CHECK(ret, hk_store_u16_set, HK_BROADCAST_KEY_GLOBAL_STATE_AT_CREATION, global_state);

    hk_mem_free(accessory_public_key);

    return ret;
}

esp_err_t hk_broadcast_key_get(hk_mem *accessory_shared_secret, hk_mem *broadcast_key)
{
    esp_err_t ret = ESP_OK;
    uint16_t global_state = hk_global_state_get();
    uint16_t global_state_at_creation = 0;

    ret = hk_store_u16_get(HK_BROADCAST_KEY_GLOBAL_STATE_AT_CREATION, &global_state_at_creation);
    if (ret == ESP_OK)
    {
        ret = hk_store_blob_get(HK_BROADCAST_KEY, broadcast_key);
    }

    if (ret != ESP_OK && ret != ESP_ERR_NOT_FOUND)
    {
        HK_LOGE("Error retrieving broadcast key.");
        return ret;
    }

    if (ret == ESP_ERR_NOT_FOUND || global_state_at_creation - global_state > 32767)
    {
        if (accessory_shared_secret == NULL)
        {
            return ESP_ERR_INVALID_ARG;
        }

        ret = ESP_OK;
        RUN_AND_CHECK(ret, hk_broadcast_key_create, accessory_shared_secret, broadcast_key);
    }

    return ret;
}

esp_err_t hk_broadcast_key_reset(hk_mem *accessory_shared_secret, hk_mem *broadcast_key)
{
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_broadcast_key_create, accessory_shared_secret, broadcast_key);
    return ret;
}