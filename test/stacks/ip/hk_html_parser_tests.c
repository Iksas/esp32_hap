// #include "unity.h"

// #include "../../../src/utils/hk_logging.h"
// #include "../../../src/include/hk_mem.h"


// #include "../../../src/stacks/ip/hk_html_parser.h"

// TEST_CASE("Get query value", "[html]")
// {
//     hk_mem *query = hk_mem_init();
//     hk_mem_append_string(query, "id=2.8");
//     hk_mem_append_string_terminator(query);
//     hk_mem *ids = hk_mem_init();

//     hk_html_parser_get_query_value(query, "id", ids);
//     TEST_ASSERT_GREATER_THAN_INT(0, ids->size);
//     hk_mem_free(query);
//     hk_mem_free(ids);
// }