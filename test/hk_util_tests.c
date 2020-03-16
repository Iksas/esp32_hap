#include "unity.h"

#include "../../utils/hk_util.h"
#include "../../include/hk_mem.h"


TEST_CASE("Util: get accessory id", "[util]")
{
    hk_mem* mac1 = hk_mem_create();
    hk_mem* mac2 = hk_mem_create();
    size_t ret = hk_util_get_accessory_id_serialized(mac1);

    TEST_ASSERT_FALSE(ret);
    ret = hk_util_get_accessory_id_serialized(mac2);
    TEST_ASSERT_FALSE(ret);
    TEST_ASSERT_TRUE(hk_mem_cmp(mac1,mac2));

    hk_mem_free(mac1);
    hk_mem_free(mac2);
    
    
}
