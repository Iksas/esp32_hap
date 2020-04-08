// #include "unity.h"
// #include "../../include/hk_mem.h"
// #include "../../utils/hk_store.h"

// #include <string.h>
// #include <esp_system.h>
// #include <nvs_flash.h>

// void hk_store_blob_get(const char *key, hk_mem* value);
// void hk_store_blob_set(const char *key, hk_mem* value);

// esp_err_t hk_store_bool_get(const char *key, bool* value);
// void hk_store_bool_set(const char *key, bool value);

// TEST_CASE("Store: check writing/reading string as blob.", "[store]")
// {
//     TEST_ASSERT_FALSE(hk_store_init());
//     //

//     const char *str = "my_value";
//     hk_mem *input = hk_mem_init();
//     hk_mem *result = hk_mem_init();

//     hk_mem_append_string(input, str);

//     hk_store_blob_set("my_key", input);
//     hk_store_blob_get("my_key", result);
//     TEST_ASSERT_TRUE(hk_mem_equal_str(result, "my_value"));

//     hk_mem_free(input);
//     hk_mem_free(result);

//     //
//     hk_store_free();
// }

// TEST_CASE("Store: check writing/reading byte array.", "[store]")
// {
//     TEST_ASSERT_FALSE(hk_store_init());
//     //
//     char buffer[] = {
//         0xde,
//         0xf0,
//         0x5c,
//         0xe5,
//         0x1d,
//         0x01,
//         0xe6,
//         0xcc,
//         0x3d,
//         0x6b,
//         0x34,
//         0xf7,
//         0xd8,
//         0xd5,
//         0x87,
//         0x08,
//         0x01,
//         0x68,
//         0xad,
//         0x61,
//         0x35,
//         0xd4,
//         0x9c,
//         0x9f,
//         0x65,
//         0xa3,
//         0x25,
//         0x7d,
//         0x00,
//         0x00,
//         0x04,
//         0xb4
//     };

//     hk_mem *input = hk_mem_init();
//     hk_mem *result = hk_mem_init();

//     hk_mem_append_buffer(input, buffer, 32);

//     hk_store_blob_set("my_key", input);
//     hk_store_blob_get("my_key", result);

//     TEST_ASSERT_EQUAL_MEMORY(buffer, result->ptr, 32);

//     hk_mem_free(input);
//     hk_mem_free(result);
//     //
//     hk_store_free();
// }

// TEST_CASE("Store: check writing/reading bool.", "[store]")
// {
//     TEST_ASSERT_FALSE(hk_store_init());
//     //

//     bool result;

//     hk_store_bool_set("bool1", true);
//     hk_store_bool_get("bool1", &result);
//     TEST_ASSERT_TRUE(result);

//     hk_store_bool_set("bool2", false);
//     hk_store_bool_get("bool2", &result);
//     TEST_ASSERT_FALSE(result);

//     //
//     hk_store_free();
// }

// TEST_CASE("Store: check writing/reading of keys.", "[store]")
// {
//     TEST_ASSERT_FALSE(nvs_flash_erase());
//     TEST_ASSERT_FALSE(hk_store_init());
//     //

//     const char *priv_str = "my_priv_value";
//     hk_mem *priv_in = hk_mem_init();
//     hk_mem *priv_out = hk_mem_init();
//     hk_mem_append_buffer(priv_in, (char *)priv_str, strlen(priv_str));
//     const char *pub_str = "my_pub_value";
//     hk_mem *pub_in = hk_mem_init();
//     hk_mem *pub_out = hk_mem_init();
//     hk_mem_append_buffer(pub_in, (char *)pub_str, strlen(pub_str));


//     TEST_ASSERT_FALSE(hk_store_keys_can_get()); // check uninitialized
//     hk_store_key_priv_set(priv_in);
//     hk_store_key_pub_set(pub_in);

//     TEST_ASSERT_TRUE(hk_store_keys_can_get());

//     hk_store_key_priv_get(priv_out);
//     hk_store_key_pub_get(pub_out);

//     TEST_ASSERT_TRUE(hk_mem_equal_str(pub_out, pub_str));
//     TEST_ASSERT_TRUE(hk_mem_equal_str(priv_out,priv_str));

//     hk_mem_free(priv_in);
//     hk_mem_free(priv_out);
//     hk_mem_free(pub_in);
//     hk_mem_free(pub_out);
//     //
//     hk_store_free();
// }