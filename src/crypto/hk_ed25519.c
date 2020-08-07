#include "hk_ed25519.h"
#include "hk_crypto_util.h"

#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/ed25519.h>

hk_ed25519_key_t *hk_ed25519_init()
{
    hk_ed25519_key_t *key = malloc(sizeof(hk_ed25519_key_t));
    key->internal = malloc(sizeof(ed25519_key));

    return key;
}

esp_err_t hk_ed25519_init_from_random(hk_ed25519_key_t *key)
{
    ed25519_key *wc_key = key->internal;
    WC_RNG rng;

    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_init, wc_key);
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_InitRng, &rng);
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_make_key, &rng, ED25519_KEY_SIZE, wc_key);

    wc_FreeRng(&rng);

    return ret ? ESP_FAIL : ESP_OK;
}

esp_err_t hk_ed25519_init_from_keys(hk_ed25519_key_t *key, hk_mem *private_key, hk_mem *public_key)
{
    ed25519_key *wc_key = key->internal;
    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_init, wc_key);
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_import_private_key,
                            (byte *)private_key->ptr, private_key->size,
                            (byte *)public_key->ptr, public_key->size,
                            wc_key);

    return ret ? ESP_FAIL : ESP_OK;
}

esp_err_t hk_ed25519_init_from_public_key(hk_ed25519_key_t *key, hk_mem *public_key)
{    
    ed25519_key *wc_key = key->internal;
    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_init, wc_key);
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_import_public, (byte *)public_key->ptr, public_key->size, wc_key);

    return ret ? ESP_FAIL : ESP_OK;
}

esp_err_t hk_ed25519_export_private_key(hk_ed25519_key_t *key, hk_mem *private_key)
{
    hk_mem_set(private_key, ED25519_PRV_KEY_SIZE);
    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_export_private_only,
                            (ed25519_key *)key->internal, (byte *)private_key->ptr, &private_key->size);

    return ret ? ESP_FAIL : ESP_OK;
}

esp_err_t hk_ed25519_export_public_key(hk_ed25519_key_t *key, hk_mem *public_key)
{
    hk_mem_set(public_key, ED25519_PUB_KEY_SIZE);
    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_export_public,
                            (ed25519_key *)key->internal, (byte *)public_key->ptr, &public_key->size);
    return ret ? ESP_FAIL : ESP_OK;
}

esp_err_t hk_ed25519_sign(hk_ed25519_key_t *key, hk_mem *message, hk_mem *signature)
{
    hk_mem_set(signature, ED25519_SIG_SIZE);
    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_sign_msg,
                            (byte *)message->ptr, message->size,
                            (byte *)signature->ptr, &signature->size,
                            (ed25519_key *)key->internal);

    return ret ? ESP_FAIL : ESP_OK;
}

esp_err_t hk_ed25519_verify(hk_ed25519_key_t *key, hk_mem *signature, hk_mem *message)
{
    int verified;

    int ret = 0;
    HK_CRYPTO_RUN_AND_CHECK(ret, wc_ed25519_verify_msg,
                            (byte *)signature->ptr, signature->size,
                            (byte *)message->ptr, message->size,
                            &verified, (ed25519_key *)key->internal);

    return ret || !verified ? ESP_FAIL : ESP_OK;
}

void hk_ed25519_free(hk_ed25519_key_t *key)
{
    if (key->internal != NULL)
    {
        wc_ed25519_free((ed25519_key *)key->internal);
        free(key->internal);
        key->internal = NULL;
    }

    free(key);
}