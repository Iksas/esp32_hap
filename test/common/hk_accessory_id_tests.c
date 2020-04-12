#include "unity.h"

#include "../../src/common/hk_accessory_id.h"
#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_store.h"

TEST_CASE("Get accessory id.", "[accessory_id]")
{
    //prepare
    hk_store_init();
    hk_mem *accessory_id_1 = hk_mem_init();
    hk_mem *accessory_id_2 = hk_mem_init();
    hk_mem *accessory_id_3 = hk_mem_init();

    //test
    TEST_ASSERT_EQUAL(ESP_OK, hk_accessory_id_reset());
    TEST_ASSERT_EQUAL(ESP_OK, hk_accessory_id_get(accessory_id_1));
    TEST_ASSERT_EQUAL(ESP_OK, hk_accessory_id_reset());
    TEST_ASSERT_EQUAL(ESP_OK, hk_accessory_id_get(accessory_id_2));
    TEST_ASSERT_EQUAL(ESP_OK, hk_accessory_id_get(accessory_id_3));

    // assert
    TEST_ASSERT_FALSE(hk_mem_equal(accessory_id_1, accessory_id_2));
    TEST_ASSERT_TRUE(hk_mem_equal(accessory_id_2, accessory_id_3));

    //cleanup
    hk_mem_free(accessory_id_1);
    hk_mem_free(accessory_id_2);
    hk_mem_free(accessory_id_3);
    hk_store_free();
}