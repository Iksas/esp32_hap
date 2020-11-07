
#include <nvs_flash.h>
#include "unity.h"
#include <string.h>
#include <esp_system.h>

#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_store.h"

TEST_CASE("Check writing/reading of keys.", "[store]")
{
    TEST_ASSERT_FALSE(nvs_flash_erase());
    TEST_ASSERT_FALSE(hk_store_init());
    const char *priv_str = "my_priv_value";
    hk_mem *priv_in = hk_mem_init();
    hk_mem *priv_out = hk_mem_init();
    hk_mem_append_buffer(priv_in, (char *)priv_str, strlen(priv_str));
    const char *pub_str = "my_pub_value";
    hk_mem *pub_in = hk_mem_init();
    hk_mem *pub_out = hk_mem_init();
    hk_mem_append_buffer(pub_in, (char *)pub_str, strlen(pub_str));

    TEST_ASSERT_FALSE(hk_store_keys_can_get()); // check uninitialized
    hk_store_key_priv_set(priv_in);
    hk_store_key_pub_set(pub_in);

    TEST_ASSERT_TRUE(hk_store_keys_can_get());

    hk_store_key_priv_get(priv_out);
    hk_store_key_pub_get(pub_out);

    TEST_ASSERT_TRUE(hk_mem_equal_str(pub_out, pub_str));
    TEST_ASSERT_TRUE(hk_mem_equal_str(priv_out, priv_str));

    //clean
    hk_mem_free(priv_in);
    hk_mem_free(priv_out);
    hk_mem_free(pub_in);
    hk_mem_free(pub_out);
    hk_store_free();
}

TEST_CASE("Checking reading/writing u8.", "[store]")
{
    TEST_ASSERT_FALSE(nvs_flash_erase());
    TEST_ASSERT_FALSE(hk_store_init());
    const char *key = "key";
    uint8_t number = 123;
    uint8_t result = -1;

    esp_err_t ret = hk_store_u8_get(key, &result);
    TEST_ASSERT_EQUAL_INT32(HK_STORE_ERR_NOT_FOUND, ret);
    ret = hk_store_u8_set(key, number);
    TEST_ASSERT_EQUAL_INT32(ESP_OK, ret);
    ret = hk_store_u8_get(key, &result);
    TEST_ASSERT_EQUAL_INT32(ESP_OK, ret);
    TEST_ASSERT_EQUAL_UINT8(number, result);

    //clean
    hk_store_free();
}