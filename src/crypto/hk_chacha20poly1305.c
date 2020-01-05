#include "hk_chacha20poly1305.h"
#include "hk_crypto_logging.h"
#include "../utils/hk_logging.h"

#define WOLFSSL_USER_SETTINGS
#define WOLFCRYPT_HAVE_SRP
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/chacha20_poly1305.h>

size_t hk_chacha20poly1305_encrypt(hk_mem *key, const char *nonce, hk_mem *message, hk_mem *encrypted)
{
    hk_mem_set(encrypted, message->size + CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE);
    return hk_chacha20poly1305_encrypt_buffer(key, nonce, NULL, 0, message->ptr, encrypted->ptr, message->size);
}

size_t hk_chacha20poly1305_encrypt_buffer(hk_mem *key, const char *nonce, char *aad, size_t aad_size, char *message, char *encrypted, size_t message_size)
{
    int ret = wc_ChaCha20Poly1305_Encrypt(
        (byte *)key->ptr,
        (byte *)nonce, (byte *)aad, aad_size,
        (byte *)message, message_size,
        (byte *)encrypted, (byte *)(encrypted + message_size));

    if (ret)
        HK_CRYPOT_ERR("Error encrypting message", ret);

    return ret;
}

size_t hk_chacha20poly1305_decrypt(hk_mem *key, const char *nonce, hk_mem *encrypted, hk_mem *message)
{
    hk_mem_set(message, encrypted->size - CHACHA20_POLY1305_AEAD_AUTHTAG_SIZE);
    return hk_chacha20poly1305_decrypt_buffer(key, (char *)nonce, NULL, 0, encrypted->ptr, message->ptr, message->size);
}

size_t hk_chacha20poly1305_decrypt_buffer(hk_mem *key, char *nonce, char *aad, size_t aad_size, char *encrypted,
                                          char *message, size_t message_size)
{
    int ret = wc_ChaCha20Poly1305_Decrypt(
        (byte *)key->ptr,
        (byte *)nonce,
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