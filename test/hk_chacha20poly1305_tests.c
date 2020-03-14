#include "unity.h"

#include "../crypto/hk_curve25519.h"
#include "../crypto/hk_chacha20poly1305.h"
#include "../../include/hk_mem.h"


TEST_CASE("export key", "[crypto, curve25519]")
{
    
    hk_curve25519_key_t *curve = hk_curve25519_init_key();
    hk_mem *key = hk_mem_create();
    hk_mem *message = hk_mem_create();
    hk_mem *encrypted = hk_mem_create();
    hk_mem *decrypted = hk_mem_create();
    const char* message_string = "my message";
    hk_mem_append_string(message, message_string);

    TEST_ASSERT_FALSE(hk_curve25519_generate_key(curve));
    TEST_ASSERT_FALSE(hk_curve25519_export_public_key(curve, key));
    TEST_ASSERT_FALSE(hk_chacha20poly1305_encrypt(key, HK_CHACHA_VERIFY_MSG2, message, encrypted));
    TEST_ASSERT_FALSE(hk_chacha20poly1305_decrypt(key, HK_CHACHA_VERIFY_MSG2, encrypted, decrypted));
    TEST_ASSERT_TRUE(hk_mem_cmp_str(decrypted, message_string));

    hk_curve25519_free_key(curve);
    hk_mem_free(key);
    hk_mem_free(message);
    hk_mem_free(encrypted);
    hk_mem_free(decrypted);
    
}