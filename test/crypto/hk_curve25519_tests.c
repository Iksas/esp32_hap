// #include "unity.h"

// #include "../crypto/hk_curve25519.h"
// #include "../../include/hk_mem.h"


// TEST_CASE("generate key", "[crypto, curve25519]")
// {
    
//     hk_curve25519_key_t *key = hk_curve25519_init_key();

//     TEST_ASSERT_FALSE(hk_curve25519_generate_key(key));

//     hk_curve25519_free_key(key);
    
// }

// TEST_CASE("free ininitialized key", "[crypto, curve25519]")
// {
    
//     hk_curve25519_key_t *key = hk_curve25519_init_key();

//     hk_curve25519_free_key(key);
    
// }

// TEST_CASE("export key", "[crypto, curve25519]")
// {
    
//     hk_curve25519_key_t *key = hk_curve25519_init_key();
//     hk_mem *public_key = hk_mem_create();

//     TEST_ASSERT_FALSE(hk_curve25519_generate_key(key));
//     TEST_ASSERT_FALSE(hk_curve25519_export_public_key(key, public_key));

//     hk_curve25519_free_key(key);
//     hk_mem_free(public_key);
    
// }

// TEST_CASE("create shared secret key", "[crypto, curve25519]")
// {
    
//     hk_curve25519_key_t *key1 = hk_curve25519_init_key();
//     hk_curve25519_key_t *key2 = hk_curve25519_init_key();
//     hk_mem *shared_secret = hk_mem_create();

//     TEST_ASSERT_FALSE(hk_curve25519_generate_key(key1));
//     TEST_ASSERT_FALSE(hk_curve25519_generate_key(key2));
//     TEST_ASSERT_FALSE(hk_curve25519_shared_secret(key1, key2, shared_secret));

//     hk_curve25519_free_key(key1);
//     hk_curve25519_free_key(key2);
//     hk_mem_free(shared_secret);
    
// }