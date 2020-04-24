// #include "unity.h"

// #include "../../include/hk_mem.h"
// #include "../../utils/hk_logging.h"

// #include "../hk_accessories.h"
// #include "../hk_accessories_store.h"
// #include "../hk_chrs.h"
// #include "../hk_session.h"
// #include <cJSON.h>

// void hk_accessories_free();
// char *hk_chrs_get_next_id_pair(char *ids, int *result);

// TEST_CASE("Chrs: parse query with one id", "[chrs]")
// {
//     const char *ids = "1.9";
//     int results[2];

//     ids = hk_chrs_get_next_id_pair((char*)ids, results);

//     TEST_ASSERT_EQUAL_INT(1, results[0]);
//     TEST_ASSERT_EQUAL_INT(9, results[1]);
//     TEST_ASSERT_NULL(ids);
// }

// TEST_CASE("Chrs: parse query with multiple id", "[chrs]")
// {
//     const char *ids = "1.9,2.3";
//     int results[2];

//     ids = hk_chrs_get_next_id_pair((char*)ids, results);

//     TEST_ASSERT_EQUAL_INT(1, results[0]);
//     TEST_ASSERT_EQUAL_INT(9, results[1]);

//     ids = hk_chrs_get_next_id_pair((char*)ids, results);

//     TEST_ASSERT_EQUAL_INT(2, results[0]);
//     TEST_ASSERT_EQUAL_INT(3, results[1]);
//     TEST_ASSERT_NULL(ids); 
// }
