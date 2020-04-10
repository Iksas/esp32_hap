#include "unity.h"

#include "../../src/crypto/hk_ed25519.h"
#include "../../src/include/hk_mem.h"

TEST_CASE("Generate key", "[crypto] [ed25519]")
{
    hk_ed25519_key_t* key = hk_ed25519_init();
    size_t ret = hk_ed25519_update_from_random(key);
    
    TEST_ASSERT_FALSE(ret);

    hk_ed25519_free(key);
}

TEST_CASE("Free ininitialized key", "[crypto] [ed25519]")
{
    hk_ed25519_key_t *key = hk_ed25519_init();

    hk_ed25519_free(key);
}

TEST_CASE("Sign and verify", "[crypto] [ed25519]")
{
    hk_ed25519_key_t *key = hk_ed25519_init();
    hk_mem* signature = hk_mem_init();
    hk_mem* message = hk_mem_init();
    hk_mem_append_string(message, "my message");

    TEST_ASSERT_FALSE(hk_ed25519_update_from_random(key));
    TEST_ASSERT_FALSE(hk_ed25519_sign(key, message, signature));
    TEST_ASSERT_FALSE(hk_ed25519_verify(key, signature, message));

    hk_ed25519_free(key);
    hk_mem_free(signature);
    hk_mem_free(message);
}

TEST_CASE("Sign and verify with export and import of public key", "[crypto] [ed25519]")
{
    hk_ed25519_key_t *key = hk_ed25519_init();
    hk_ed25519_key_t *key_imported = hk_ed25519_init();
    hk_mem* signature = hk_mem_init();
    hk_mem* message = hk_mem_init();
    hk_mem* public_key = hk_mem_init();
    hk_mem_append_string(message, "my message");

    TEST_ASSERT_FALSE(hk_ed25519_update_from_random(key));
    TEST_ASSERT_FALSE(hk_ed25519_sign(key, message, signature));
    TEST_ASSERT_FALSE(hk_ed25519_export_public_key(key, public_key));

    TEST_ASSERT_FALSE(hk_ed25519_update_from_random_from_public_key(key_imported, public_key));
    TEST_ASSERT_FALSE(hk_ed25519_verify(key_imported, signature, message));

    hk_ed25519_free(key);
    hk_ed25519_free(key_imported);
    hk_mem_free(public_key);
    hk_mem_free(signature);
    hk_mem_free(message);
}

TEST_CASE("Sign and verify with export and import of public and private keys", "[crypto] [ed25519]")
{
    hk_ed25519_key_t *key = hk_ed25519_init();
    hk_ed25519_key_t *key_imported = hk_ed25519_init();
    hk_ed25519_key_t *key_imported_public = hk_ed25519_init();
    hk_mem* public_key = hk_mem_init();
    hk_mem* private_key = hk_mem_init();
    hk_mem* signature = hk_mem_init();
    hk_mem* message = hk_mem_init();
    hk_mem_append_string(message, "my message");

    TEST_ASSERT_FALSE(hk_ed25519_update_from_random(key));
    TEST_ASSERT_FALSE(hk_ed25519_export_public_key(key, public_key));
    TEST_ASSERT_FALSE(hk_ed25519_export_private_key(key, private_key));

    TEST_ASSERT_FALSE(hk_ed25519_update_from_random_keys(key_imported, private_key, public_key));
    TEST_ASSERT_FALSE(hk_ed25519_sign(key_imported, message, signature));

    TEST_ASSERT_FALSE(hk_ed25519_update_from_random_from_public_key(key_imported_public, public_key));
    TEST_ASSERT_FALSE(hk_ed25519_verify(key_imported_public, signature, message));

    hk_ed25519_free(key);
    hk_ed25519_free(key_imported);
    hk_ed25519_free(key_imported_public);
    hk_mem_free(public_key);
    hk_mem_free(private_key);
    hk_mem_free(signature);
    hk_mem_free(message);
}
