#include "hk_srp.h"
#include "hk_crypto_logging.h"
#include "../utils/hk_logging.h"

#define WOLFSSL_USER_SETTINGS
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha512.h>
#include <wolfssl/wolfcrypt/srp.h>

// 3072-bit group N (per RFC5054, Appendix A)
const byte hk_srp_n[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc9, 0x0f, 0xda, 0xa2,
    0x21, 0x68, 0xc2, 0x34, 0xc4, 0xc6, 0x62, 0x8b, 0x80, 0xdc, 0x1c, 0xd1,
    0x29, 0x02, 0x4e, 0x08, 0x8a, 0x67, 0xcc, 0x74, 0x02, 0x0b, 0xbe, 0xa6,
    0x3b, 0x13, 0x9b, 0x22, 0x51, 0x4a, 0x08, 0x79, 0x8e, 0x34, 0x04, 0xdd,
    0xef, 0x95, 0x19, 0xb3, 0xcd, 0x3a, 0x43, 0x1b, 0x30, 0x2b, 0x0a, 0x6d,
    0xf2, 0x5f, 0x14, 0x37, 0x4f, 0xe1, 0x35, 0x6d, 0x6d, 0x51, 0xc2, 0x45,
    0xe4, 0x85, 0xb5, 0x76, 0x62, 0x5e, 0x7e, 0xc6, 0xf4, 0x4c, 0x42, 0xe9,
    0xa6, 0x37, 0xed, 0x6b, 0x0b, 0xff, 0x5c, 0xb6, 0xf4, 0x06, 0xb7, 0xed,
    0xee, 0x38, 0x6b, 0xfb, 0x5a, 0x89, 0x9f, 0xa5, 0xae, 0x9f, 0x24, 0x11,
    0x7c, 0x4b, 0x1f, 0xe6, 0x49, 0x28, 0x66, 0x51, 0xec, 0xe4, 0x5b, 0x3d,
    0xc2, 0x00, 0x7c, 0xb8, 0xa1, 0x63, 0xbf, 0x05, 0x98, 0xda, 0x48, 0x36,
    0x1c, 0x55, 0xd3, 0x9a, 0x69, 0x16, 0x3f, 0xa8, 0xfd, 0x24, 0xcf, 0x5f,
    0x83, 0x65, 0x5d, 0x23, 0xdc, 0xa3, 0xad, 0x96, 0x1c, 0x62, 0xf3, 0x56,
    0x20, 0x85, 0x52, 0xbb, 0x9e, 0xd5, 0x29, 0x07, 0x70, 0x96, 0x96, 0x6d,
    0x67, 0x0c, 0x35, 0x4e, 0x4a, 0xbc, 0x98, 0x04, 0xf1, 0x74, 0x6c, 0x08,
    0xca, 0x18, 0x21, 0x7c, 0x32, 0x90, 0x5e, 0x46, 0x2e, 0x36, 0xce, 0x3b,
    0xe3, 0x9e, 0x77, 0x2c, 0x18, 0x0e, 0x86, 0x03, 0x9b, 0x27, 0x83, 0xa2,
    0xec, 0x07, 0xa2, 0x8f, 0xb5, 0xc5, 0x5d, 0xf0, 0x6f, 0x4c, 0x52, 0xc9,
    0xde, 0x2b, 0xcb, 0xf6, 0x95, 0x58, 0x17, 0x18, 0x39, 0x95, 0x49, 0x7c,
    0xea, 0x95, 0x6a, 0xe5, 0x15, 0xd2, 0x26, 0x18, 0x98, 0xfa, 0x05, 0x10,
    0x15, 0x72, 0x8e, 0x5a, 0x8a, 0xaa, 0xc4, 0x2d, 0xad, 0x33, 0x17, 0x0d,
    0x04, 0x50, 0x7a, 0x33, 0xa8, 0x55, 0x21, 0xab, 0xdf, 0x1c, 0xba, 0x64,
    0xec, 0xfb, 0x85, 0x04, 0x58, 0xdb, 0xef, 0x0a, 0x8a, 0xea, 0x71, 0x57,
    0x5d, 0x06, 0x0c, 0x7d, 0xb3, 0x97, 0x0f, 0x85, 0xa6, 0xe1, 0xe4, 0xc7,
    0xab, 0xf5, 0xae, 0x8c, 0xdb, 0x09, 0x33, 0xd7, 0x1e, 0x8c, 0x94, 0xe0,
    0x4a, 0x25, 0x61, 0x9d, 0xce, 0xe3, 0xd2, 0x26, 0x1a, 0xd2, 0xee, 0x6b,
    0xf1, 0x2f, 0xfa, 0x06, 0xd9, 0x8a, 0x08, 0x64, 0xd8, 0x76, 0x02, 0x73,
    0x3e, 0xc8, 0x6a, 0x64, 0x52, 0x1f, 0x2b, 0x18, 0x17, 0x7b, 0x20, 0x0c,
    0xbb, 0xe1, 0x17, 0x57, 0x7a, 0x61, 0x5d, 0x6c, 0x77, 0x09, 0x88, 0xc0,
    0xba, 0xd9, 0x46, 0xe2, 0x08, 0xe2, 0x4f, 0xa0, 0x74, 0xe5, 0xab, 0x31,
    0x43, 0xdb, 0x5b, 0xfc, 0xe0, 0xfd, 0x10, 0x8e, 0x4b, 0x82, 0xd1, 0x20,
    0xa9, 0x3a, 0xd2, 0xca, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

// 3072-bit group generator (per RFC5054, Appendix A)
const byte hk_srp_g[] = {0x05};

hk_srp_key_t *hk_srp_init_key()
{
    hk_srp_key_t *key = malloc(sizeof(hk_srp_key_t));
    key->internal = malloc(sizeof(Srp));

    return key;
}

void hk_random_fill(uint8_t *data, size_t size)
{
    uint16_t x = esp_random();
    memcpy(data, &x, sizeof(uint8_t));
}

size_t hk_srp_generate_key(hk_srp_key_t *key, const char *username, const char *password)
{
    byte salt[16];
    hk_random_fill(salt, sizeof(salt));

    size_t ret;

    // We initialize srp as client, though we are server. But in order to calculate the
    // verifier, wolf ssl needs to be in client mode.
    ret = wc_SrpInit((Srp *)key->internal, SRP_TYPE_SHA512, SRP_CLIENT_SIDE);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to initialize SRP", ret);
        return ret;
    }

    ret = wc_SrpSetUsername((Srp *)key->internal, (byte *)username, strlen(username));
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to set SRP username", ret);
        return ret;
    }

    ret = wc_SrpSetParams((Srp *)key->internal, hk_srp_n, sizeof(hk_srp_n), hk_srp_g, sizeof(hk_srp_g), salt, sizeof(salt));
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to set SRP params", ret);
        return ret;
    }

    ret = wc_SrpSetPassword((Srp *)key->internal, (byte *)password, strlen(password));
    if (ret)
    {
        HK_CRYPOT_ERR("Srp: Failed to set SRP password (code %d)", ret);
        return ret;
    }

    size_t verifierLen = 1024;
    byte *verifier = malloc(verifierLen);
    ret = wc_SrpGetVerifier((Srp *)key->internal, verifier, &verifierLen);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to get SRP verifier", ret);
        free(verifier);
        return ret;
    }

    ((Srp *)key->internal)->side = SRP_SERVER_SIDE; // now switch to server side
    ret = wc_SrpSetVerifier((Srp *)key->internal, verifier, verifierLen);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to set SRP verifier", ret);
        free(verifier);
        return ret;
    }

    free(verifier);

    return ret;
}

size_t hk_srp_export_public_key(hk_srp_key_t *key, hk_mem *public_key)
{
    hk_mem_set(public_key, 384);

    size_t ret = wc_SrpGetPublic((Srp *)key->internal, (byte *)public_key->ptr, &public_key->size);
    if (ret)
    {
        HK_CRYPOT_ERR("Error getting public key", ret);
    }

    return ret;
}

size_t hk_srp_export_private_key(hk_srp_key_t *key, hk_mem *private_key)
{
    hk_mem_append_buffer(private_key, (char *)((Srp *)key->internal)->key, ((Srp *)key->internal)->keySz);
    return ESP_OK;
}

size_t hk_srp_export_salt(hk_srp_key_t *key, hk_mem *salt)
{
    hk_mem_append_buffer(salt, (char *)((Srp *)key->internal)->salt, ((Srp *)key->internal)->saltSz);
    return ESP_OK;
}

size_t hk_srp_verify(hk_srp_key_t *key, hk_mem *proof)
{
    size_t ret = wc_SrpVerifyPeersProof((Srp *)key->internal, (byte *)proof->ptr, proof->size);
    if (ret == SRP_VERIFY_E)
    {
        HK_LOGW("Error verifying client proof. Maybe on IOS entered password was wrong. Error: %d", ret);
    }
    else if (ret)
    {
        HK_CRYPOT_ERR("Error verifying client proof", ret);
    }

    return ret;
}

int hk_srp_set_key(Srp *srp, byte *secret, word32 size)
{
    SrpHash hash;
    size_t ret = ESP_OK;

    srp->key = (byte *)XMALLOC(WC_SHA512_DIGEST_SIZE, NULL, DYNAMIC_TYPE_SRP);
    if (!srp->key)
    {
        HK_CRYPOT_ERR("Error allocating key", ret);
        return -1;
    }

    srp->keySz = WC_SHA512_DIGEST_SIZE;

    ret = wc_InitSha512(&hash.data.sha512);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to initialize generation of Sha512", ret);
        return ret;
    }

    ret = wc_Sha512Update(&hash.data.sha512, secret, size);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to update Sha512", ret);
        return ret;
    }

    ret = wc_Sha512Final(&hash.data.sha512, srp->key);
    if (ret)
    {
        HK_CRYPOT_ERR("Failed to finalize Sha512", ret);
        return ret;
    }

    memset(&hash, 0, sizeof(hash));

    return ret;
}

size_t hk_srp_compute_key(hk_srp_key_t *key, hk_mem *server_public_key, hk_mem *client_public_key)
{
    ((Srp *)key->internal)->keyGenFunc_cb = hk_srp_set_key;
    size_t ret = wc_SrpComputeKey(
        (Srp *)key->internal,
        (byte *)client_public_key->ptr, client_public_key->size,
        (byte *)server_public_key->ptr, server_public_key->size);

    if (ret)
    {
        HK_CRYPOT_ERR("Error computing key", ret);
    }

    return ret;
}

size_t hk_srp_export_proof(hk_srp_key_t *key, hk_mem *proof)
{
    hk_mem_set(proof, WC_SHA512_DIGEST_SIZE);
    int ret = wc_SrpGetProof((Srp *)key->internal, (byte *)proof->ptr, &proof->size);
    if (ret)
    {
        HK_CRYPOT_ERR("Error getting server proof", ret);
    }

    return ret;
}

void hk_srp_free_key(hk_srp_key_t *key)
{
    if (key->internal != NULL)
    {
        wc_SrpTerm((Srp *)key->internal);
        free(key->internal);
        key->internal = NULL;
    }

    free(key);
}