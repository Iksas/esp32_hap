#include "hk_hkdf.h"
#include "hk_crypto_logging.h"
#include "../utils/hk_logging.h"

#define WOLFSSL_USER_SETTINGS
#define WOLFCRYPT_HAVE_SRP
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha512.h>
#include <wolfssl/wolfcrypt/hmac.h>

esp_err_t static hk_hkdf_internal(
    char *key_in, size_t key_in_length,
    char *key_out, size_t key_out_length,
    char *salt, size_t salt_length,
    const char *info, size_t info_length)
{
    HK_LOGV("Deriving an encrption key with salt/info: %s %s", salt, info);
    int ret = wc_HKDF(
        SHA512,
        (byte *)key_in, key_in_length,
        (byte *)salt, salt_length,
        (byte *)info, info_length,
        (byte *)key_out, key_out_length);

    if (ret != 0)
    {
        HK_CRYPOT_ERR("Error deriving key", ret);
        return ESP_FAIL;
    }
    else
    {
        return ESP_OK;
    }
}

size_t hk_hkdf(hk_mem *key_in, hk_mem *key_out, const char* salt, const char* info)
{
    return hk_hkdf_with_given_size(key_in, key_out, 32, salt, info);
}

size_t hk_hkdf_with_given_size(hk_mem *key_in, hk_mem *key_out, size_t size, const char* salt, const char* info)
{
    hk_mem_set(key_out, size);

    return hk_hkdf_internal(
        key_in->ptr, key_in->size,
        key_out->ptr, key_out->size,
        (char*)salt, strlen(salt),
        info, strlen(info));
}

size_t hk_hkdf_with_external_salt(hk_mem *key_in, hk_mem *key_out, hk_mem *salt, const char* info)
{
    hk_mem_set(key_out, 32);
    
    return hk_hkdf_internal(
        key_in->ptr, key_in->size,
        key_out->ptr, key_out->size,
        salt->ptr, salt->size,
        info, strlen(info));
}
