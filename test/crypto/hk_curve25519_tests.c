#include "unity.h"

#include "../../src/crypto/hk_curve25519.h"
#include "../../src/include/hk_mem.h"

TEST_CASE("Generate key", "[crypto] [curve25519]")
{
    hk_curve25519_key_t *key = hk_curve25519_init();

    TEST_ASSERT_FALSE(hk_curve25519_update_from_random(key));

    hk_curve25519_free(key);
}

TEST_CASE("Free ininitialized key", "[crypto, curve25519]")
{
    hk_curve25519_key_t *key = hk_curve25519_init();

    hk_curve25519_free(key);
}

TEST_CASE("Export key", "[crypto] [curve25519]")
{
    hk_curve25519_key_t *key = hk_curve25519_init();
    hk_mem *public_key = hk_mem_init();

    TEST_ASSERT_FALSE(hk_curve25519_update_from_random(key));
    TEST_ASSERT_FALSE(hk_curve25519_export_public_key(key, public_key));

    hk_curve25519_free(key);
    hk_mem_free(public_key);
}

TEST_CASE("Create shared secret key", "[crypto] [curve25519]")
{
    hk_curve25519_key_t *key1 = hk_curve25519_init();
    hk_curve25519_key_t *key2 = hk_curve25519_init();
    hk_mem *shared_secret = hk_mem_init();

    TEST_ASSERT_FALSE(hk_curve25519_update_from_random(key1));
    TEST_ASSERT_FALSE(hk_curve25519_update_from_random(key2));
    TEST_ASSERT_FALSE(hk_curve25519_calculate_shared_secret(key1, key2, shared_secret));

    hk_curve25519_free(key1);
    hk_curve25519_free(key2);
    hk_mem_free(shared_secret);
}