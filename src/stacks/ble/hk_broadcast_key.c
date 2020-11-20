#include "hk_broadcast_key.h"

#include "../../crypto/hk_hkdf.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_global_state.h"

hk_mem *hk_broadcast_key = NULL;
uint16_t hk_broadcast_key_global_state_at_creation = 0;

esp_err_t hk_broadcast_key_create(hk_mem *accessory_shared_secret)
{
    esp_err_t ret = ESP_OK;
    hk_mem *accessory_public_key = hk_mem_init();

    if (hk_broadcast_key == NULL)
    {
        hk_broadcast_key = hk_mem_init();
    }
    else
    {
        hk_mem_set(hk_broadcast_key, 0);
    }

    // get public key and keys->shared_secret
    RUN_AND_CHECK(ret, hk_store_key_pub_get, accessory_public_key);

    // generate broadcast_key
    RUN_AND_CHECK(ret, hk_hkdf_with_external_salt, accessory_shared_secret, hk_broadcast_key, accessory_public_key, HK_HKDF_BROADCAST_ENCRYPTION_KEY_INFO);

    hk_mem_free(accessory_public_key);

    hk_broadcast_key_global_state_at_creation = hk_global_state_get();

    return ret;
}

esp_err_t hk_broadcast_key_get(hk_mem *accessory_shared_secret, hk_mem *broadcast_key)
{
    esp_err_t ret = ESP_OK;
    uint16_t global_state = hk_global_state_get();

    if (hk_broadcast_key == NULL || hk_broadcast_key_global_state_at_creation - global_state > 32767)
    {
        if (accessory_shared_secret == NULL)
        {
            return ESP_ERR_INVALID_ARG;
        }

        RUN_AND_CHECK(ret, hk_broadcast_key_create, accessory_shared_secret);
    }

    hk_mem_set(broadcast_key, 0);
    hk_mem_append(broadcast_key, hk_broadcast_key);

    return ret;
}

esp_err_t hk_broadcast_key_reset(hk_mem *accessory_shared_secret)
{
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_broadcast_key_create, accessory_shared_secret);
    return ret;
}