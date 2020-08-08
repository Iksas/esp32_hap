#include "unity.h"

#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_store.h"
#include "../../src/utils/hk_logging.h"


#include "../../src/common/hk_pairings_store.h"

TEST_CASE("Add first", "[pair] [store]")
{
    // prepare
    TEST_ASSERT_FALSE(hk_store_init());
    hk_pairings_store_remove_all();
    hk_mem *device_id = hk_mem_init();
    hk_mem_append_string(device_id, "my_device_id");
    hk_mem *device_ltpk = hk_mem_init();
    hk_mem_append_string(device_ltpk, "my_device_ltpk");
    bool device_exists = false;

    // test
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id, device_ltpk, true));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_device_exists(device_id, &device_exists));
    TEST_ASSERT_TRUE(device_exists);

    // cleanup
    hk_mem_free(device_id);
    hk_mem_free(device_ltpk);
    hk_store_free();
}

TEST_CASE("Get ltpk", "[pair] [store]")
{
    // prepare
    TEST_ASSERT_FALSE(hk_store_init());
    hk_pairings_store_remove_all();
    hk_mem *device_id = hk_mem_init();
    hk_mem_append_string(device_id, "my_device_id");
    hk_mem *device_ltpk = hk_mem_init();
    hk_mem_append_string(device_ltpk, "my_device_ltpk");
    hk_mem *device_ltpk_result = hk_mem_init();

    // test
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id, device_ltpk, true));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_ltpk_get(device_id, device_ltpk_result));
    TEST_ASSERT_TRUE(hk_mem_equal(device_ltpk, device_ltpk_result));

    // cleanup
    hk_mem_free(device_id);
    hk_mem_free(device_ltpk);
    hk_mem_free(device_ltpk_result);
    hk_store_free();
}

TEST_CASE("Remove pairing", "[pair] [store]")
{
    // prepare
    TEST_ASSERT_FALSE(hk_store_init());
    hk_pairings_store_remove_all();
    hk_mem *device_id = hk_mem_init();
    hk_mem_append_string(device_id, "my_device_id");
    hk_mem *device_ltpk = hk_mem_init();
    hk_mem_append_string(device_ltpk, "my_device_ltpk");
    hk_mem *device_ltpk_result = hk_mem_init();
    bool device_exists = false;

    // test
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id, device_ltpk, true));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_remove(device_id));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_device_exists(device_id, &device_exists));
    TEST_ASSERT_FALSE(device_exists);

    // cleanup
    hk_mem_free(device_id);
    hk_mem_free(device_ltpk);
    hk_mem_free(device_ltpk_result);
    hk_store_free();
}

TEST_CASE("hk_pairings_store_has_admin_pairing", "[pair] [store]")
{
    // prepare
    TEST_ASSERT_FALSE(hk_store_init());
    hk_pairings_store_remove_all();
    hk_mem *device_id1 = hk_mem_init();
    hk_mem_append_string(device_id1, "my_device_id1");
    hk_mem *device_ltpk1 = hk_mem_init();
    hk_mem_append_string(device_ltpk1, "my_device_ltpk1");
    hk_mem *device_id2 = hk_mem_init();
    hk_mem_append_string(device_id2, "my_device_id2");
    hk_mem *device_ltpk2 = hk_mem_init();
    hk_mem_append_string(device_ltpk2, "my_device_ltpk2");
    bool has_admin = true;

    // test
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id1, device_ltpk1, false));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_has_admin_pairing(&has_admin));
    TEST_ASSERT_FALSE(has_admin);
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id2, device_ltpk2, true));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_has_admin_pairing(&has_admin));
    TEST_ASSERT_TRUE(has_admin);

    // cleanup
    hk_mem_free(device_id1);
    hk_mem_free(device_ltpk1);
    hk_mem_free(device_id2);
    hk_mem_free(device_ltpk2);
    hk_store_free();
}

TEST_CASE("hk_pairings_store_is_admin", "[pair] [store]")
{
    // prepare
    TEST_ASSERT_FALSE(hk_store_init());
    hk_pairings_store_remove_all();
    hk_mem *device_id1 = hk_mem_init();
    hk_mem_append_string(device_id1, "one");
    hk_mem *device_ltpk1 = hk_mem_init();
    hk_mem_append_string(device_ltpk1, "one_ltpk");
    hk_mem *device_id2 = hk_mem_init();
    hk_mem_append_string(device_id2, "two");
    hk_mem *device_ltpk2 = hk_mem_init();
    hk_mem_append_string(device_ltpk2, "two_ltpk");
    bool is_admin = true;

    // test
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id1, device_ltpk1, false));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_is_admin(device_id1, &is_admin));
    TEST_ASSERT_FALSE(is_admin);
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_add(device_id2, device_ltpk2, true));
    TEST_ASSERT_EQUAL(ESP_OK, hk_pairings_store_is_admin(device_id2, &is_admin));
    TEST_ASSERT_TRUE(is_admin);

    // cleanup
    hk_mem_free(device_id1);
    hk_mem_free(device_ltpk1);
    hk_mem_free(device_id2);
    hk_mem_free(device_ltpk2);
    hk_store_free();
}
