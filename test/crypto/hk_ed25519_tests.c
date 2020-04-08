// #include "unity.h"

// #include "../crypto/hk_ed25519.h"
// #include "../../include/hk_mem.h"


// TEST_CASE("Ed25519: generate key", "[crypto, ed25519]")
// {
    
//     hk_ed25519_key_t* key = hk_ed25519_init_key();
//     size_t ret = hk_ed25519_generate_key(key);
    
//     TEST_ASSERT_FALSE(ret);

//     hk_ed25519_free_key(key);
    
// }

// TEST_CASE("Ed25519: free ininitialized key", "[crypto, ed25519]")
// {
    
//     hk_ed25519_key_t *key = hk_ed25519_init_key();

//     hk_ed25519_free_key(key);
    
// }

// TEST_CASE("Ed25519: sign and verify", "[crypto, ed25519]")
// {
    
//     hk_ed25519_key_t *key = hk_ed25519_init_key();
//     hk_mem* signature = hk_mem_init();
//     hk_mem* message = hk_mem_init();
//     hk_mem_append_string(message, "my message");

//     TEST_ASSERT_FALSE(hk_ed25519_generate_key(key));
//     TEST_ASSERT_FALSE(hk_ed25519_sign(key, message, signature));
//     TEST_ASSERT_FALSE(hk_ed25519_verify(key, message, signature));

//     hk_ed25519_free_key(key);
//     hk_mem_free(signature);
//     hk_mem_free(message);
    
// }

// TEST_CASE("Ed25519: sign and verify with export and import of public key", "[crypto, ed25519]")
// {
    
//     hk_ed25519_key_t *key = hk_ed25519_init_key();
//     hk_ed25519_key_t *key_imported = hk_ed25519_init_key();
//     hk_mem* signature = hk_mem_init();
//     hk_mem* message = hk_mem_init();
//     hk_mem* public_key = hk_mem_init();
//     hk_mem_append_string(message, "my message");

//     TEST_ASSERT_FALSE(hk_ed25519_generate_key(key));
//     TEST_ASSERT_FALSE(hk_ed25519_sign(key, message, signature));
//     TEST_ASSERT_FALSE(hk_ed25519_export_public_key(key, public_key));

//     TEST_ASSERT_FALSE(hk_ed25519_generate_key_from_public_key(key_imported, public_key));
//     TEST_ASSERT_FALSE(hk_ed25519_verify(key_imported, message, signature));

//     hk_ed25519_free_key(key);
//     hk_ed25519_free_key(key_imported);
//     hk_mem_free(public_key);
//     hk_mem_free(signature);
//     hk_mem_free(message);
    
// }

// TEST_CASE("Ed25519: sign and verify with export and import of public and private keys", "[crypto, ed25519]")
// {
    
//     hk_ed25519_key_t *key = hk_ed25519_init_key();
//     hk_ed25519_key_t *key_imported = hk_ed25519_init_key();
//     hk_ed25519_key_t *key_imported_public = hk_ed25519_init_key();
//     hk_mem* public_key = hk_mem_init();
//     hk_mem* private_key = hk_mem_init();
//     hk_mem* signature = hk_mem_init();
//     hk_mem* message = hk_mem_init();
//     hk_mem_append_string(message, "my message");

//     TEST_ASSERT_FALSE(hk_ed25519_generate_key(key));
//     TEST_ASSERT_FALSE(hk_ed25519_export_public_key(key, public_key));
//     TEST_ASSERT_FALSE(hk_ed25519_export_private_key(key, private_key));

//     TEST_ASSERT_FALSE(hk_ed25519_generate_key_from_keys(key_imported, private_key, public_key));
//     TEST_ASSERT_FALSE(hk_ed25519_sign(key_imported, message, signature));

//     TEST_ASSERT_FALSE(hk_ed25519_generate_key_from_public_key(key_imported_public, public_key));
//     TEST_ASSERT_FALSE(hk_ed25519_verify(key_imported_public, message, signature));

//     hk_ed25519_free_key(key);
//     hk_ed25519_free_key(key_imported);
//     hk_ed25519_free_key(key_imported_public);
//     hk_mem_free(public_key);
//     hk_mem_free(private_key);
//     hk_mem_free(signature);
//     hk_mem_free(message);
    
// }
