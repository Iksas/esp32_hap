// #include "unity.h"

// #include "../../utils/hk_logging.h"
// #include "../../utils/hk_mem.h"


// #include "../hk_accessories_store.h"
// #include "../hk_accessories.h"

// void hk_accessories_free();

// TEST_CASE("Accessories: check getting chr", "[accessories]")
// {
//     hk_accessories_store_add_accessory();
//     hk_accessories_store_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false);
//     hk_accessories_store_add_chr_static_read(HK_CHR_NAME, (void *)"name");
//     hk_accessories_store_add_chr_static_read(HK_CHR_MANUFACTURER, (void *)"manufacturer");
//     hk_accessories_store_add_srv(HK_SRV_AIR_PURIFIER, true, false);
//     hk_accessories_store_end_config();

    

//     hk_chr_t *c = hk_accessories_store_get_chr(1,2);
//     TEST_ASSERT_NOT_NULL(c);
//     HK_LOGD("Found c %x, %x, %x",c->type, HK_CHR_NAME, HK_CHR_MANUFACTURER);
//     TEST_ASSERT_EQUAL_INT(2, c->iid);
//     TEST_ASSERT_EQUAL_INT(HK_CHR_NAME, c->type);

//     c = hk_accessories_store_get_chr(1,4);
//     TEST_ASSERT_NULL(c);

//     c = hk_accessories_store_get_chr(2,4);
//     TEST_ASSERT_NULL(c);

    
//     hk_accessories_free();
// }
