#include "hk_ed25519.h"
#include "../utils/hk_res.h"
#include "hk_crypto_logging.h"

#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/ed25519.h>

hk_ed25519_key_t *hk_ed25519_init()
{
    hk_ed25519_key_t *key = malloc(sizeof(hk_ed25519_key_t));
    key->internal = malloc(sizeof(ed25519_key));
    
    return key;
}

esp_err_t hk_ed25519_generate_key(hk_ed25519_key_t *key)
{
    ed25519_key *k = key->internal;
    esp_err_t ret = wc_ed25519_init(k);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed initializing key", ret);
        return ret;
    }

    WC_RNG rng;
    ret = wc_InitRng(&rng);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to create random for key", ret);
        return ret;
    }

    ret = wc_ed25519_make_key(&rng, ED25519_KEY_SIZE, k);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to make key", ret);
        return ret;
    }

    wc_FreeRng(&rng);

    return ret;
}

esp_err_t hk_ed25519_generate_key_from_keys(hk_ed25519_key_t *key, hk_mem *private_key, hk_mem *public_key)
{
    ed25519_key *k = key->internal;
    esp_err_t ret = wc_ed25519_init(k);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed initializing key", ret);
        return ret;
    }

    ret = wc_ed25519_import_private_key(
        (byte *)private_key->ptr, private_key->size,
        (byte *)public_key->ptr, public_key->size,
        k);
    if (ret)
        HK_CRYPOT_ERR("Failed importing device public key", ret);

    return ret;
}

esp_err_t hk_ed25519_generate_key_from_public_key(hk_ed25519_key_t *key, hk_mem *public_key)
{
    ed25519_key *k = key->internal;
    esp_err_t ret = wc_ed25519_init(k);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed initializing key", ret);
        return ret;
    }

    ret = wc_ed25519_import_public((byte *)public_key->ptr, public_key->size, k);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed importing device public key", ret);
    }

    return ret;
}

esp_err_t hk_ed25519_export_private_key(hk_ed25519_key_t *key, hk_mem *private_key)
{
    hk_mem_set(private_key, ED25519_PRV_KEY_SIZE);
    esp_err_t ret = wc_ed25519_export_private_only((ed25519_key *)key->internal, (byte *)private_key->ptr, &private_key->size);

    if (ret)
    {
        HK_CRYPOT_ERR("Failed exporting key", ret);
    }

    return ret;
}

esp_err_t hk_ed25519_export_public_key(hk_ed25519_key_t *key, hk_mem *public_key)
{
    hk_mem_set(public_key, ED25519_PUB_KEY_SIZE);
    esp_err_t ret = wc_ed25519_export_public((ed25519_key *)key->internal, (byte *)public_key->ptr, &public_key->size);

    if (ret)
    {
        HK_CRYPOT_ERR("Failed exporting public key", ret);
    }

    return ret;
}

esp_err_t hk_ed25519_sign(hk_ed25519_key_t *key, hk_mem *message, hk_mem *signature)
{
    hk_mem_set(signature, ED25519_SIG_SIZE);
    esp_err_t ret = wc_ed25519_sign_msg(
        (byte *)message->ptr, message->size,
        (byte *)signature->ptr, &signature->size,
        (ed25519_key *)key->internal);

    if (ret)
    {
        HK_CRYPOT_ERR("Failed to sign message", ret);
    }

    return ret;
}

esp_err_t hk_ed25519_verify(hk_ed25519_key_t *key, hk_mem *signature, hk_mem *message)
{
    int verified;

    esp_err_t ret = wc_ed25519_verify_msg(
        (byte *)signature->ptr, signature->size,
        (byte *)message->ptr, message->size,
        &verified, (ed25519_key *)key->internal);

    if (!verified)
    {
        HK_LOGE("Message not valid.");
    }
    else if (ret != HK_RES_OK)
    {
        HK_CRYPOT_ERR("Failed to verify", ret);
    }

    return ret || !verified;
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