// #include "unity.h"

// #include "../../utils/hk_mem.h"
// #include "../../utils/hk_logging.h"
// #include "../../utils/hk_res.h"

// #include "../hk_accessories.h"
// #include "../hk_accessories_store.h"
// #include "../hk_characteristics.h"
// #include "../hk_session.h"
// #include <cJSON.h>

// void hk_accessories_free();
// char *hk_characteristics_get_next_id_pair(char *ids, int *result);

// TEST_CASE("Characteristics: parse query with one id", "[characteristics]")
// {
//     const char *ids = "1.9";
//     int results[2];

//     ids = hk_characteristics_get_next_id_pair((char*)ids, results);

//     TEST_ASSERT_EQUAL_INT(1, results[0]);
//     TEST_ASSERT_EQUAL_INT(9, results[1]);
//     TEST_ASSERT_NULL(ids);
// }

// TEST_CASE("Characteristics: parse query with multiple id", "[characteristics]")
// {
//     const char *ids = "1.9,2.3";
//     int results[2];

//     ids = hk_characteristics_get_next_id_pair((char*)ids, results);

//     TEST_ASSERT_EQUAL_INT(1, results[0]);
//     TEST_ASSERT_EQUAL_INT(9, results[1]);

//     ids = hk_characteristics_get_next_id_pair((char*)ids, results);

//     TEST_ASSERT_EQUAL_INT(2, results[0]);
//     TEST_ASSERT_EQUAL_INT(3, results[1]);
//     TEST_ASSERT_NULL(ids); 
// }
