
#include <nvs_flash.h>
#include "unity.h"
#include <string.h>
#include <esp_system.h>

#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_store.h"

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