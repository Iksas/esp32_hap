#include "unity.h"

#include "../../utils/hk_mem.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_logging.h"


#include "../hk_pairings_store.h"

TEST_CASE("Pairings store: add to empty", "[pairings_store]")
{
    TEST_ASSERT_FALSE(hk_store_init());
    //

    hk_pairings_store_remove_all();
    hk_mem *device_id = hk_mem_create();
    hk_mem_append_string(device_id, "my_device_id");
    hk_mem *device_ltpk = hk_mem_create();
    hk_mem_append_string(device_ltpk, "my_device_ltpk");

    hk_pairings_store_add(device_id, device_ltpk, true);

    hk_mem_free(device_id);
    hk_mem_free(device_ltpk);

    //
    hk_store_free();
}

TEST_CASE("Pairings store: get ltpk", "[pairings_store]")
{
    TEST_ASSERT_FALSE(hk_store_init());
    //

    hk_pairings_store_remove_all();
    hk_mem *device_id = hk_mem_create();
    hk_mem_append_string(device_id, "my_device_id");
    hk_mem *device_ltpk = hk_mem_create();
    hk_mem_append_string(device_ltpk, "my_device_ltpk");
    hk_mem *device_ltpk_result = hk_mem_create();

    hk_pairings_store_add(device_id, device_ltpk, true);
    esp_err_t ret = hk_pairings_store_ltpk_get(device_id, device_ltpk_result);

    TEST_ASSERT_FALSE(ret);
    TEST_ASSERT_TRUE(hk_mem_cmp(device_ltpk, device_ltpk_result));

    hk_mem_free(device_id);
    hk_mem_free(device_ltpk);
    hk_mem_free(device_ltpk_result);

    //
    hk_store_free();
}

TEST_CASE("Pairings store: remove pairing", "[pairings_store]")
{
    TEST_ASSERT_FALSE(hk_store_init());
    //

    hk_pairings_store_remove_all();
    hk_mem *device_id = hk_mem_create();
    hk_mem_append_string(device_id, "my_device_id");
    hk_mem *device_ltpk = hk_mem_create();
    hk_mem_append_string(device_ltpk, "my_device_ltpk");
    hk_mem *device_ltpk_result = hk_mem_create();

    hk_pairings_store_add(device_id, device_ltpk, true);
    esp_err_t ret = hk_pairings_store_ltpk_get(device_id, device_ltpk_result);

    TEST_ASSERT_FALSE(ret);

    hk_pairings_store_remove(device_id);
    ret = hk_pairings_store_ltpk_get(device_id, device_ltpk_result);

    TEST_ASSERT_TRUE(ret);

    hk_mem_free(device_id);
    hk_mem_free(device_ltpk);
    hk_mem_free(device_ltpk_result);

    //
    hk_store_free();
}

TEST_CASE("Pairings store: check for admin pairing", "[pairings_store]")
{
    TEST_ASSERT_FALSE(hk_store_init());
    //

    hk_pairings_store_remove_all();
    hk_mem *device_id1 = hk_mem_create();
    hk_mem_append_string(device_id1, "my_device_id1");
    hk_mem *device_ltpk1 = hk_mem_create();
    hk_mem_append_string(device_ltpk1, "my_device_ltpk1");
    hk_mem *device_id2 = hk_mem_create();
    hk_mem_append_string(device_id2, "my_device_id2");
    hk_mem *device_ltpk2 = hk_mem_create();
    hk_mem_append_string(device_ltpk2, "my_device_ltpk2");

    hk_pairings_store_add(device_id1, device_ltpk1, false);
    TEST_ASSERT_FALSE(hk_pairings_store_has_admin_pairing());
    hk_pairings_store_add(device_id2, device_ltpk2, true);
    TEST_ASSERT_TRUE(hk_pairings_store_has_admin_pairing());

    hk_mem_free(device_id1);
    hk_mem_free(device_ltpk1);
    hk_mem_free(device_id2);
    hk_mem_free(device_ltpk2);

    //
    hk_store_free();
}

TEST_CASE("Pairings store: check for admin pairing with device id", "[pairings_store]")
{
    TEST_ASSERT_FALSE(hk_store_init());
    //

    hk_pairings_store_remove_all();
    hk_mem *device_id1 = hk_mem_create();
    hk_mem_append_string(device_id1, "my_device_id1");
    hk_mem *device_ltpk1 = hk_mem_create();
    hk_mem_append_string(device_ltpk1, "my_device_ltpk1");
    hk_mem *device_id2 = hk_mem_create();
    hk_mem_append_string(device_id2, "my_device_id2");
    hk_mem *device_ltpk2 = hk_mem_create();
    hk_mem_append_string(device_ltpk2, "my_device_ltpk2");

    hk_pairings_store_add(device_id1, device_ltpk1, false);
    hk_pairings_store_add(device_id2, device_ltpk2, true);

    TEST_ASSERT_FALSE(hk_pairings_store_is_admin(device_id1));
    TEST_ASSERT_TRUE(hk_pairings_store_is_admin(device_id2));

    hk_mem_free(device_id1);
    hk_mem_free(device_ltpk1);
    hk_mem_free(device_id2);
    hk_mem_free(device_ltpk2);

    //
    hk_store_free();
}
