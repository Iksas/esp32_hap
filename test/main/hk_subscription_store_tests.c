#include "unity.h"

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"
#include "../hk_subscription_store.h"
#include "../hk_session.h"
#include "../hk_accessories_store.h"

TEST_CASE("Subscriptions: add subscription", "[subscriptions]")
{
    hk_session_t *session = malloc(sizeof(hk_session_t));
    hk_characteristic_t *characteristic = malloc(sizeof(hk_characteristic_t));
    hk_subscription_store_add_session(characteristic, session);

    hk_session_t **sessions = hk_subscription_store_get_sessions(characteristic);
    hk_ll_foreach(sessions, session_item)
    {
        TEST_ASSERT_EQUAL_PTR(*session_item, session);
    }

    hk_subscription_store_free();
    free(session);
    free(characteristic);
}

TEST_CASE("Subscriptions: add subscriptions", "[subscriptions]")
{
    hk_session_t *session1 = malloc(sizeof(hk_session_t));
    hk_session_t *session2 = malloc(sizeof(hk_session_t));
    hk_characteristic_t *characteristic = malloc(sizeof(hk_characteristic_t));
    hk_subscription_store_add_session(characteristic, session1);
    hk_subscription_store_add_session(characteristic, session2);

    hk_session_t **sessions = hk_subscription_store_get_sessions(characteristic);
    int session_count = hk_ll_count(sessions);
    TEST_ASSERT_EQUAL_INT(2, session_count);
    hk_subscription_store_free();
    free(session1);
    free(session2);
    free(characteristic);
}

TEST_CASE("Subscriptions: remove session", "[subscriptions]")
{
    hk_session_t *session1 = malloc(sizeof(hk_session_t));
    hk_session_t *session2 = malloc(sizeof(hk_session_t));
    hk_characteristic_t *characteristic1 = malloc(sizeof(hk_characteristic_t));
    hk_characteristic_t *characteristic2 = malloc(sizeof(hk_characteristic_t));
    hk_subscription_store_add_session(characteristic1, session1);
    hk_subscription_store_add_session(characteristic1, session2);
    hk_subscription_store_add_session(characteristic2, session2);

    hk_subscription_store_remove_session(session2);

    hk_session_t **sessions = hk_subscription_store_get_sessions(characteristic1);
    int session_count = hk_ll_count(sessions);
    TEST_ASSERT_EQUAL_INT(1, session_count);

    sessions = hk_subscription_store_get_sessions(characteristic2);
    session_count = hk_ll_count(sessions);
    TEST_ASSERT_EQUAL_INT(0, session_count);

    hk_subscription_store_free();
    free(session1);
    free(session2);
    free(characteristic1);
    free(characteristic2);
}

TEST_CASE("Subscriptions: mix", "[subscriptions]")
{
    hk_session_t *session1 = malloc(sizeof(hk_session_t));
    hk_session_t *session2 = malloc(sizeof(hk_session_t));
    hk_characteristic_t *characteristic1 = malloc(sizeof(hk_characteristic_t));
    hk_characteristic_t *characteristic2 = malloc(sizeof(hk_characteristic_t));
    hk_subscription_store_add_session(characteristic1, session1);
    hk_subscription_store_add_session(characteristic1, session2);
    hk_subscription_store_add_session(characteristic2, session2);

    hk_subscription_store_remove_session(session2);
    hk_subscription_store_add_session(characteristic1, session2);
    hk_subscription_store_add_session(characteristic2, session2);
    hk_subscription_store_remove_session(session1);
    hk_subscription_store_add_session(characteristic1, session1);
    hk_subscription_store_remove_session_from_subscription(characteristic1, session2);
    hk_subscription_store_add_session(characteristic1, session2);

    hk_session_t **sessions = hk_subscription_store_get_sessions(characteristic1);
    int session_count = hk_ll_count(sessions);
    TEST_ASSERT_EQUAL_INT(2, session_count);

    sessions = hk_subscription_store_get_sessions(characteristic2);
    session_count = hk_ll_count(sessions);
    TEST_ASSERT_EQUAL_INT(1, session_count);

    HK_LOGI("1");
    hk_subscription_store_remove_session_from_subscription(characteristic1, session2);
    HK_LOGI("2");
    hk_subscription_store_remove_session_from_subscription(characteristic1, session2);
    HK_LOGI("3");
    hk_subscription_store_remove_session_from_subscription(characteristic1, session1);
    HK_LOGI("4");
    hk_subscription_store_remove_session(session1);
    HK_LOGI("5");
    hk_subscription_store_remove_session(session1);
    HK_LOGI("6");

    sessions = hk_subscription_store_get_sessions(characteristic1);
    session_count = hk_ll_count(sessions);
    TEST_ASSERT_EQUAL_INT(0, session_count);

    hk_subscription_store_free();
    free(session1);
    free(session2);
    free(characteristic1);
    free(characteristic2);
}
