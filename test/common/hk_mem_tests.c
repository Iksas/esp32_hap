#include "unity.h"
#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_heap_debug.h"

#include <string.h>
#include <esp_heap_caps.h>

TEST_CASE("Check multiple append.", "[mem]")
{
    hk_heap_debug_start();
    size_t freeMemBefore = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    const char *test_data = "testData";
    for (size_t i = 0; i < 1000; i++)
    {
        hk_mem *hm = hk_mem_init();
        hk_mem_append_buffer(hm, (char *)test_data, strlen(test_data));
        hk_mem_free(hm);
    }

    size_t freeMemAfter = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    TEST_ASSERT_EQUAL(freeMemBefore, freeMemAfter);
    TEST_ASSERT_TRUE(heap_caps_check_integrity_all(true))
    hk_heap_debug_stop();
}

TEST_CASE("Check multiple append string.", "[mem]")
{
    hk_heap_debug_start();
    const char *test_data = "testData";

    hk_mem *hm = hk_mem_init();

    hk_mem_append_string(hm, test_data);
    TEST_ASSERT_EQUAL_INT(strlen(test_data), hm->size);

    hk_mem_append_string(hm, test_data);
    TEST_ASSERT_EQUAL_INT(strlen(test_data) * 2, hm->size);

    hk_mem_free(hm);
    hk_heap_debug_stop();
}