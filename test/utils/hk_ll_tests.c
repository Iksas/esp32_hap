#include "unity.h"

#include <esp_system.h>
#include "../../src/utils/hk_ll.h"
#include "../../src/utils/hk_heap_debug.h"

typedef struct
{
    int data;
    int *data_ptr;
} hk_ll_test_t;

TEST_CASE("create and free", "[ll]")
{
    hk_heap_debug_start();
    hk_ll_test_t *list = NULL;

    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);

    TEST_ASSERT_EQUAL_INT(3, hk_ll_count(list));

    hk_ll_free(list);
    hk_heap_debug_stop();
}

TEST_CASE("iterate", "[ll]")
{
    hk_ll_test_t *list = NULL;

    list = hk_ll_init(list);
    list->data = 0;

    list = hk_ll_init(list);
    list->data = 1;

    list = hk_ll_init(list);
    list->data = 2;

    int count = 2;
    hk_ll_foreach(list, item)
    {
        TEST_ASSERT_EQUAL_INT(count, item->data);
        count--;
    }
    TEST_ASSERT_EQUAL_INT(-1, count);

    hk_ll_free(list);
}

TEST_CASE("reverse", "[ll]")
{

    hk_ll_test_t *list = NULL;

    list = hk_ll_init(list);
    list->data = 0;

    list = hk_ll_init(list);
    list->data = 1;

    list = hk_ll_init(list);
    list->data = 2;

    list = hk_ll_reverse(list);

    int count = -1;
    hk_ll_foreach(list, item)
    {
        count++;
        TEST_ASSERT_EQUAL_INT(count, item->data);
    }
    TEST_ASSERT_EQUAL_INT(2, count);

    hk_ll_free(list);
}

TEST_CASE("remove first", "[ll]")
{
    hk_heap_debug_start();
    hk_ll_test_t *list = NULL;

    list = hk_ll_init(list);
    list->data = 0;

    list = hk_ll_init(list);
    list->data = 1;
    hk_ll_test_t *item1 = list;

    list = hk_ll_init(list);
    list->data = 2;
    hk_ll_test_t *item2 = list;

    list = hk_ll_remove(list, item2);
    TEST_ASSERT_EQUAL_INT(2, hk_ll_count(list));
    TEST_ASSERT_EQUAL_INT(item1, list);
    TEST_ASSERT_EQUAL_INT(item1->data, list->data);

    hk_ll_free(list);
    hk_heap_debug_stop();
}

TEST_CASE("remove middle", "[ll]")
{

    hk_ll_test_t *list = NULL;

    list = hk_ll_init(list);
    list->data = 0;
    hk_ll_test_t *item0 = list;

    list = hk_ll_init(list);
    list->data = 1;
    hk_ll_test_t *item1 = list;

    list = hk_ll_init(list);
    list->data = 2;
    hk_ll_test_t *item2 = list;

    list = hk_ll_remove(list, item1);

    TEST_ASSERT_EQUAL_INT(2, hk_ll_count(list));
    TEST_ASSERT_EQUAL_INT(item2, list);

    hk_ll_test_t *item = list;
    TEST_ASSERT_EQUAL_INT(item2->data, item->data);

    item = hk_ll_next(list);
    TEST_ASSERT_EQUAL_INT(item0->data, item->data);

    hk_ll_free(list);
}

TEST_CASE("remove last", "[ll]")
{

    hk_ll_test_t *list = NULL;

    list = hk_ll_init(list);
    list->data = 0;
    hk_ll_test_t *item0 = list;

    list = hk_ll_init(list);
    list->data = 1;
    hk_ll_test_t *item1 = list;

    list = hk_ll_init(list);
    list->data = 2;
    hk_ll_test_t *item2 = list;

    list = hk_ll_remove(list, item0);

    TEST_ASSERT_EQUAL_INT(2, hk_ll_count(list));
    TEST_ASSERT_EQUAL_INT(item2, list);

    hk_ll_test_t *item = list;
    TEST_ASSERT_EQUAL_INT(item2->data, item->data);

    item = hk_ll_next(list);
    TEST_ASSERT_EQUAL_INT(item1->data, item->data);

    hk_ll_free(list);
}

TEST_CASE("remove odd in iteration", "[ll]")
{
    hk_ll_test_t *list = NULL;
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);

    hk_ll_test_t *expected_list_after_delete = list;
    int count = 0;
    int count_deleted = 0;
    for (hk_ll_test_t *item = list; item != NULL;)
    {
        if (count % 2)
        {
            hk_ll_test_t *next = hk_ll_next(item);
            list = hk_ll_remove(list, item);
            item = next;
            count_deleted++;
        }
        else
        {
            item = hk_ll_next(item);
        }
        count++;
    }

    TEST_ASSERT_EQUAL_INT(4, hk_ll_count(list));
    TEST_ASSERT_EQUAL_INT(7, count);
    TEST_ASSERT_EQUAL_INT(3, count_deleted);
    TEST_ASSERT_EQUAL_INT(expected_list_after_delete, list);

    hk_ll_free(list);
}

TEST_CASE("remove even in iteration", "[ll]")
{
    hk_ll_test_t *list = NULL;
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);
    list = hk_ll_init(list);

    hk_ll_test_t *expected_list_after_delete = hk_ll_next(list);
    int count = 0;
    int count_deleted = 0;
    for (hk_ll_test_t *item = list; item != NULL;)
    {
        if (count % 2 == 0)
        {
            hk_ll_test_t *next = hk_ll_next(item);
            list = hk_ll_remove(list, item);
            item = next;
            count_deleted++;
        }
        else
        {
            item = hk_ll_next(item);
        }
        count++;
    }

    TEST_ASSERT_EQUAL_INT(3, hk_ll_count(list));
    TEST_ASSERT_EQUAL_INT(7, count);
    TEST_ASSERT_EQUAL_INT(4, count_deleted);
    TEST_ASSERT_EQUAL_INT(expected_list_after_delete, list);

    hk_ll_free(list);
}