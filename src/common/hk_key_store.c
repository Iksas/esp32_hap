#include "hk_accessory_id.h"

#include <esp_system.h>

#include "../utils/hk_store.h"
#include "../utils/hk_util.h"

#define HK_STORE_PAIRED "hk_paired"
#define HK_STORE_ACC_PRV_KEY "hk_acc_prv_key"
#define HK_STORE_ACC_PUB_KEY "hk_acc_pub_key"

esp_err_t hk_key_store_priv_get(hk_mem *value)
{
    return hk_store_blob_get(HK_STORE_ACC_PRV_KEY, value);
}

esp_err_t hk_key_store_priv_set(hk_mem *value)
{
    return hk_store_blob_set(HK_STORE_ACC_PRV_KEY, value);
}

esp_err_t hk_key_store_pub_get(hk_mem *value)
{
    return hk_store_blob_get(HK_STORE_ACC_PUB_KEY, value);
}

esp_err_t hk_key_store_pub_set(hk_mem *value)
{
    return hk_store_blob_set(HK_STORE_ACC_PUB_KEY, value);
}