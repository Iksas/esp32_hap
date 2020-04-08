// #include "unity.h"

// #include "../../utils/hk_logging.h"
// #include "../../include/hk_mem.h"


// #include "../hk_accessories_serializer.h"
// #include "../hk_accessories.h"

// void hk_accessories_free();
// void hk_accessories_serializer_tests_dummy(void* data){}

// TEST_CASE("Accessories Serializer: check", "[accessories]")
// {
//     hk_accessories_store_add_accessory();
//     hk_accessories_store_add_srv(HK_SRV_ACCESSORY_INFORMATION, false, false);
//     hk_accessories_store_add_chr_static_read(HK_CHR_NAME, (void *)"name");
//     hk_accessories_store_add_chr_static_read(HK_CHR_MANUFACTURER, (void *)"manufacturer");
//     hk_accessories_store_add_chr_static_read(HK_CHR_MODEL, (void *)"model");
//     hk_accessories_store_add_chr_static_read(HK_CHR_SERIAL_NUMBER, (void *)"serial_number");
//     hk_accessories_store_add_chr_static_read(HK_CHR_FIRMWARE_REVISION, (void *)"revision");
//     hk_accessories_store_add_chr(HK_CHR_IDENTIFY, NULL, hk_accessories_serializer_tests_dummy, NULL);
//     hk_accessories_store_end_config();

//     hk_mem *out = hk_mem_init();
//     hk_accessories_serializer_accessories(out);
//     hk_mem_log_as_string("Accessories", out);
//     TEST_ASSERT_GREATER_THAN_INT(0, out->size);
//     hk_mem_free(out);
    
//     hk_accessories_free();
// }