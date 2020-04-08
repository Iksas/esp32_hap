#include "unity.h"

#include "../../src/utils/hk_util.h"
#include "../../src/include/hk_mem.h"


TEST_CASE("get accessory id", "[util]")
{
    hk_mem* mac1 = hk_mem_create();
    hk_mem* mac2 = hk_mem_create();
    size_t ret = hk_util_get_accessory_id_serialized(mac1);

    TEST_ASSERT_FALSE(ret);
    ret = hk_util_get_accessory_id_serialized(mac2);
    TEST_ASSERT_FALSE(ret);
    TEST_ASSERT_TRUE(hk_mem_equal(mac1,mac2));

    hk_mem_free(mac1);
    hk_mem_free(mac2);
    
    
}
