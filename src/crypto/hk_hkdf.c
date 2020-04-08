#include "hk_hkdf.h"
#include "hk_crypto_logging.h"
#include "../utils/hk_logging.h"


#define WOLFSSL_USER_SETTINGS
#define WOLFCRYPT_HAVE_SRP
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha512.h>
#include <wolfssl/wolfcrypt/hmac.h>

struct hk_hkdf_salt_info {
    char* salt;
    char* info;
};

static struct hk_hkdf_salt_info hk_hkdf_salt_infos[] = {
    {   
        .salt = "Pair-Setup-Encrypt-Salt",
        .info = "Pair-Setup-Encrypt-Info",
    },
    { 
        .salt = "Pair-Setup-Controller-Sign-Salt",
        .info = "Pair-Setup-Controller-Sign-Info",
    },
    {   
        .salt = "Pair-Setup-Accessory-Sign-Salt",
        .info = "Pair-Setup-Accessory-Sign-Info",
    },
    {   
        .salt = "Pair-Verify-Encrypt-Salt",
        .info = "Pair-Verify-Encrypt-Info",
    },
    {  
        .salt = "Pair-Verify-ResumeSessionID-Salt",
        .info = "Pair-Verify-ResumeSessionID-Info",
    },
    {  
        .salt = "external",
        .info = "Pair-Resume-Request-Info",
    },
    {  
        .salt = "external",
        .info = "Pair-Resume-Response-Info",
    },
    {  
        .salt = "external",
        .info = "Pair-Resume-Shared-Secret-Info",
    },
    {   
        .salt = "Control-Salt",
        .info = "Control-Read-Encryption-Key",
    },
    {  
        .salt = "Control-Salt",
        .info = "Control-Write-Encryption-Key",
    },
    {  
        .salt = "external",
        .info = "Broadcast-Encryption-Key",
    }
};

size_t hk_hkdf(enum hk_hkdf_types type, hk_mem *key_in, hk_mem* key_out)
{        
    return hk_hkdf_with_given_size(type, key_in, key_out, 32);
}

size_t hk_hkdf_with_given_size(enum hk_hkdf_types type, hk_mem *key_in, hk_mem* key_out, size_t size)
{
    hk_mem_set(key_out, size);

    char* salt = hk_hkdf_salt_infos[type].salt;
    char* info = hk_hkdf_salt_infos[type].info;
    HK_LOGV("Deriving an encrption key with salt/info: %s %s", salt, info);
    int ret = wc_HKDF(
        SHA512,
        (byte*)key_in->ptr, key_in->size,
        (byte *)salt, strlen(salt),
        (byte *)info, strlen(info),
        (byte*)key_out->ptr, key_out->size);
        
    return ret;
}

size_t hk_hkdf_with_external_salt(enum hk_hkdf_types type, hk_mem *salt, hk_mem *key_in, hk_mem* key_out)
{
    hk_mem_set(key_out, 32);

    char* info = hk_hkdf_salt_infos[type].info;
    hk_mem_log("salt", salt);
    HK_LOGV("Deriving an encrption key with info: %s %d", info, strlen(info));
    int ret = wc_HKDF(
        SHA512,
        (byte*)key_in->ptr, key_in->size,
        (byte *)salt->ptr, salt->size,
        (byte *)info, strlen(info),
        (byte*)key_out->ptr, key_out->size);
        
    return ret;
}
