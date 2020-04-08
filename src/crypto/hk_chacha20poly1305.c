#include "hk_chacha20poly1305.h"
#include "hk_crypto_logging.h"
#include "../utils/hk_logging.h"
#include "../include/hk_mem.h"

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/chacha20_poly1305.h>
#include <wolfssl/wolfcrypt/poly1305.h>
#include <wolfssl/wolfcrypt/chacha.h>

static void hk_chacha20poly1305_word32_to_little64(const word32 inLittle32, byte outLittle64[8])
{
    XMEMSET(outLittle64 + 4, 0, 4);

    outLittle64[0] = (byte)(inLittle32 & 0x000000FF);
    outLittle64[1] = (byte)((inLittle32 & 0x0000FF00) >> 8);
    outLittle64[2] = (byte)((inLittle32 & 0x00FF0000) >> 16);
    outLittle64[3] = (byte)((inLittle32 & 0xFF000000) >> 24);
}

esp_err_t hk_chacha20poly1305_caluclate_auth_tag_without_message(hk_mem *key, const char *nonce, hk_mem *auth_tag)
{
    if (key->size != CHACHA20_POLY1305_AEAD_KEYSIZE)
    {
        HK_LOGE("Key size has to be %d but was %d.", CHACHA20_POLY1305_AEAD_KEYSIZE, key->size);
    }

    Poly1305 poly1305Ctx;
    ChaCha chaChaCtx;
    byte poly1305Key[CHACHA20_POLY1305_AEAD_KEYSIZE];
    byte little64[16];

    XMEMSET(poly1305Key, 0, sizeof(poly1305Key));

    /* Create the Poly1305 key */
    int err = wc_Chacha_SetKey(&chaChaCtx, (const byte *)key->ptr, CHACHA20_POLY1305_AEAD_KEYSIZE);

    if (!err)
    {
        err = wc_Chacha_SetIV(&chaChaCtx, (const byte *)nonce, 0);
    }

    if (!err)
    {
        err = wc_Chacha_Process(&chaChaCtx, poly1305Key, poly1305Key, CHACHA20_POLY1305_AEAD_KEYSIZE);
    }

    if (!err)
    {
        /* Initialize Poly1305 */
        err = wc_Poly1305SetKey(&poly1305Ctx, poly1305Key, CHACHA20_POLY1305_AEAD_KEYSIZE);
    }

    if (!err)
    {
        /* -- AAD length as a 64-bit little endian integer */
        hk_chacha20poly1305_word32_to_little64(0, little64);
        /* -- Ciphertext length as a 64-bit little endian integer */
        hk_chacha20poly1305_word32_to_little64(0, little64 + 8);
        err = wc_Poly1305Update(&poly1305Ctx, little64, sizeof(little64));
    }

    if (!err)
    {

        /* Finalize the auth tag */
        hk_mem_set(auth_tag, CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE);
        err = wc_Poly1305Final(&poly1305Ctx, (byte *)auth_tag->ptr);
    }

    if (err == 0)
    {
        return ESP_OK;
    }
    else
    {
        HK_CRYPOT_ERR("Failed decrypt message", err);
        return ESP_FAIL;
    }
}

esp_err_t hk_chacha20poly1305_encrypt(hk_mem *key, const char *nonce, hk_mem *message, hk_mem *encrypted)
{
    hk_mem_set(encrypted, message->size + CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE);
    return hk_chacha20poly1305_encrypt_buffer(key, nonce, NULL, 0, message->ptr, encrypted->ptr, message->size);
}

esp_err_t hk_chacha20poly1305_encrypt_buffer(hk_mem *key, const char *nonce, char *aad, size_t aad_size, char *message, char *encrypted, size_t message_size)
{
    int ret = wc_ChaCha20Poly1305_Encrypt(
        (byte *)key->ptr,
        (byte *)nonce,
        (byte *)aad, aad_size,
        (byte *)message, message_size,
        (byte *)encrypted, (byte *)(encrypted + message_size));

    if (ret)
        HK_CRYPOT_ERR("Error encrypting message", ret);

    return ret;
}

esp_err_t hk_chacha20poly1305_decrypt(hk_mem *key, const char *nonce, hk_mem *encrypted, hk_mem *message)
{
    hk_mem_set(message, encrypted->size - CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE);
    return hk_chacha20poly1305_decrypt_buffer(key, (char *)nonce, NULL, 0, encrypted->ptr, message->ptr, message->size);
}

esp_err_t hk_chacha20poly1305_decrypt_buffer(hk_mem *key, const char *nonce, char *aad, size_t aad_size, char *encrypted,
                                          char *message, size_t message_size)
{
    int ret = wc_ChaCha20Poly1305_Decrypt(
        (byte *)key->ptr,
        (const byte *)nonce,
        (byte *)aad, aad_size,
        (byte *)encrypted, message_size,  // the encrypted message
        (byte *)encrypted + message_size, // the authTag
        (byte *)message);

    if (ret)
    {
        HK_CRYPOT_ERR("Failed decrypt message", ret);
    }

    return ret;
}

esp_err_t hk_chacha20poly1305_verify_auth_tag(hk_mem *key, const char *nonce, hk_mem *auth_tag)
{
    hk_mem *calculated_auth_tag = hk_mem_init();
    esp_err_t ret = hk_chacha20poly1305_caluclate_auth_tag_without_message(key, nonce, calculated_auth_tag);

    if (ret != ESP_OK)
    {
        return ret;
    }

    if (!hk_mem_equal(auth_tag, calculated_auth_tag))
    {
        return ESP_ERR_INVALID_MAC;
    }

    return ESP_OK;
}