#include "unity.h"
#include "../../include/hk_mem.h"

#include <string.h>
#include <esp_heap_caps.h>

TEST_CASE("Mem: check multiple append.", "[mem]")
{
    
    size_t freeMemBefore = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    const char *test_data = "testData";
    for (size_t i = 0; i < 1000; i++)
    {
        hk_mem *hm = hk_mem_create();
        hk_mem_append_buffer(hm, (char *)test_data, strlen(test_data));
        hk_mem_free(hm);
    }

    size_t freeMemAfter = heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    TEST_ASSERT_EQUAL(freeMemBefore, freeMemAfter);
    TEST_ASSERT_TRUE(heap_caps_check_integrity_all(true))
    
}

TEST_CASE("Mem: check multiple append string.", "[mem]")
{
    
    const char *test_data = "testData";

    hk_mem *hm = hk_mem_create();

    hk_mem_append_string(hm, test_data);
    TEST_ASSERT_EQUAL_INT(strlen(test_data), hm->size);
    
    hk_mem_append_string(hm, test_data);
    TEST_ASSERT_EQUAL_INT(strlen(test_data) * 2, hm->size);

    hk_mem_free(hm);
    
}