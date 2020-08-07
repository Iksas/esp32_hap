
#include <string.h>
#include <esp_system.h>
#include <unity.h>

#include "../../src/utils/hk_tlv.h"
#include "../../src/utils/hk_logging.h"
#include "../../src/include/hk_mem.h"
#include "../../src/utils/hk_heap_debug.h"

hk_tlv_t* append_string(hk_tlv_t *tlv, const char* str, char type){
    hk_mem *mem = hk_mem_init();
    hk_mem_append_buffer(mem, (char*)str, strlen(str));

    HK_LOGI("hk_tlv_add");
    tlv = hk_tlv_add(tlv, type, mem);
    hk_mem_free(mem);
    return tlv;
}

TEST_CASE("full run", "[tlv]")
{
    hk_heap_debug_start();
    HK_LOGI("Memory before: %d", memory_before);
    
    hk_tlv_t *input_tlv = NULL;
    hk_tlv_t *output_tlv = NULL;
    hk_mem *data = hk_mem_init();
    hk_mem *item = hk_mem_init();

    input_tlv = append_string(input_tlv, "bla1", 1);
    input_tlv = append_string(input_tlv, "bla2", 2);
    input_tlv = append_string(input_tlv, "bla3", 3);
    input_tlv = append_string(input_tlv, "bla4", 4);

    HK_LOGI("hk_tlv_serialize");
    hk_tlv_serialize(input_tlv, data);

    HK_LOGI("hk_tlv_deserialize");
    output_tlv = hk_tlv_deserialize(data);

    HK_LOGI("hk_tlv_get_mem_by_type");
    hk_tlv_get_mem_by_type(output_tlv, 4, item);

    TEST_ASSERT_TRUE(hk_mem_equal_str(item, "bla4"));

    hk_mem_free(data);
    hk_mem_free(item);
    HK_LOGI("hk_tlv_free");
    hk_tlv_free(input_tlv);
    HK_LOGI("hk_tlv_free");
    hk_tlv_free(output_tlv);

    hk_heap_debug_stop();   
}

TEST_CASE("simple add", "[tlv]")
{
    
    hk_tlv_t *tlv = NULL;

    const char* str = "bla";
    hk_mem *mem = hk_mem_init();
    hk_mem_append_buffer(mem, (char*)str, strlen(str));

    tlv = hk_tlv_add(tlv, 1, mem);
    hk_tlv_free(tlv);
    hk_mem_free(mem);
    
}
