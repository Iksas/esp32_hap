#include "unity.h"

#include "../../../src/utils/hk_util.h"
#include "../../../src/stacks/ip/hk_subscription_store.h"
#include "../../../src/stacks/ip/hk_accessories_store.h"

TEST_CASE("add first subscription", "[subscriptions]")
{
    // setup
    hk_chr_t *chr = malloc(sizeof(hk_chr_t));
    int socket = 123;

    // execute
    esp_err_t ret = hk_subscription_store_add(chr, socket);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(1, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(socket, sockets[0]);

    // clean up
    hk_subscription_store_free();
    free(chr);
}

TEST_CASE("add second socket to subscription", "[subscriptions]")
{
    // setup
    hk_chr_t *chr = malloc(sizeof(hk_chr_t));
    int socket1 = 123;
    int socket2 = 124;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr, socket1);

    // execute
    ret = hk_subscription_store_add(chr, socket2);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(2, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(socket1, sockets[0]);
    TEST_ASSERT_EQUAL_INT(socket2, sockets[1]);

    // clean up
    hk_subscription_store_free();
    free(chr);
}

TEST_CASE("add second subscription", "[subscriptions]")
{
    // setup
    hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
    int socket1 = 123;
    int socket2 = 124;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, socket1);

    // execute
    ret = hk_subscription_store_add(chr2, socket2);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr2, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(1, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(socket2, sockets[0]);

    // clean up
    hk_subscription_store_free();
    free(chr1);
    free(chr2);
}

TEST_CASE("delete first subscription", "[subscriptions]")
{
    // setup
    esp_err_t ret = ESP_OK;
    int socket = 123;
    hk_chr_t *chr = malloc(sizeof(hk_chr_t));
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr, socket);

    // execute
    ret = hk_subscription_store_remove(chr, socket);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(0, number_of_sockets);

    // clean up
    hk_subscription_store_free();
    free(chr);
}

TEST_CASE("remove subscription", "[subscriptions]")
{
    // setup
    hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
    int socket = 123;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 124);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 125);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 126);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 127);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 128);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 129);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, socket);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 130);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 131);

    // execute
    ret = hk_subscription_store_remove(chr2, socket);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr2, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(5, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(127, sockets[0]);
    TEST_ASSERT_EQUAL_INT(128, sockets[1]);
    TEST_ASSERT_EQUAL_INT(129, sockets[2]);
    TEST_ASSERT_EQUAL_INT(130, sockets[3]);
    TEST_ASSERT_EQUAL_INT(131, sockets[4]);

    // clean up
    hk_subscription_store_free();
    free(chr1);
    free(chr2);
}

TEST_CASE("remove subscription where list does not exist.", "[subscriptions]")
{
    // setup
    hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr3 = malloc(sizeof(hk_chr_t));
    int socket = 123;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 124);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 125);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 126);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 127);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 128);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 129);

    // execute
    ret = hk_subscription_store_remove(chr2, socket);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    // clean up
    hk_subscription_store_free();
    free(chr1);
    free(chr2);
    free(chr3);
}

TEST_CASE("remove subscription where socket is not in list.", "[subscriptions]")
{
    // setup
    hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr3 = malloc(sizeof(hk_chr_t));
    int socket = 123;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 124);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 125);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 126);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 127);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 128);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 129);

    // execute
    ret = hk_subscription_store_remove(chr2, socket);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);

    // clean up
    hk_subscription_store_free();
    free(chr1);
    free(chr2);
    free(chr3);
}

TEST_CASE("remove a socket from all lists.", "[subscriptions]")
{
    // setup
    hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
    hk_chr_t *chr3 = malloc(sizeof(hk_chr_t));
    int socket = 123;
    esp_err_t ret = ESP_OK;
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 124);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, socket);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr1, 126);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 127);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, socket);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr2, 129);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr3, 127);
    RUN_AND_CHECK(ret, hk_subscription_store_add, chr3, 129);

    // execute
    ret = hk_subscription_store_remove_all(socket);

    // assert
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    int *sockets = NULL;
    size_t number_of_sockets = -1;
    ret = hk_subscription_store_get(chr1, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(2, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(124, sockets[0]);
    TEST_ASSERT_EQUAL_INT(126, sockets[1]);
    sockets = NULL;
    number_of_sockets = -1;
    ret = hk_subscription_store_get(chr2, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(2, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(127, sockets[0]);
    TEST_ASSERT_EQUAL_INT(129, sockets[1]);
    sockets = NULL;
    number_of_sockets = -1;
    ret = hk_subscription_store_get(chr3, &sockets, &number_of_sockets);
    TEST_ASSERT_EQUAL_INT(ESP_OK, ret);
    TEST_ASSERT_EQUAL_INT(2, number_of_sockets);
    TEST_ASSERT_EQUAL_INT(127, sockets[0]);
    TEST_ASSERT_EQUAL_INT(129, sockets[1]);

    // clean up
    hk_subscription_store_free();
    free(chr1);
    free(chr2);
    free(chr3);
}

// TEST_CASE("add subscriptions", "[subscriptions]")
// {
//     hk_session_t *session1 = malloc(sizeof(hk_session_t));
//     hk_session_t *session2 = malloc(sizeof(hk_session_t));
//     hk_chr_t *chr = malloc(sizeof(hk_chr_t));
//     hk_subscription_store_add_session(chr, session1);
//     hk_subscription_store_add_session(chr, session2);

//     hk_session_t **sessions = hk_subscription_store_get_sessions(chr);
//     int session_count = hk_ll_count(sessions);
//     TEST_ASSERT_EQUAL_INT(2, session_count);
//     hk_subscription_store_free();
//     free(session1);
//     free(session2);
//     free(chr);
// }

// TEST_CASE("remove session", "[subscriptions]")
// {
//     hk_session_t *session1 = malloc(sizeof(hk_session_t));
//     hk_session_t *session2 = malloc(sizeof(hk_session_t));
//     hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
//     hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
//     hk_subscription_store_add_session(chr1, session1);
//     hk_subscription_store_add_session(chr1, session2);
//     hk_subscription_store_add_session(chr2, session2);

//     hk_subscription_store_remove_session(session2);

//     hk_session_t **sessions = hk_subscription_store_get_sessions(chr1);
//     int session_count = hk_ll_count(sessions);
//     TEST_ASSERT_EQUAL_INT(1, session_count);

//     sessions = hk_subscription_store_get_sessions(chr2);
//     session_count = hk_ll_count(sessions);
//     TEST_ASSERT_EQUAL_INT(0, session_count);

//     hk_subscription_store_free();
//     free(session1);
//     free(session2);
//     free(chr1);
//     free(chr2);
// }

// TEST_CASE("mix", "[subscriptions]")
// {
//     hk_session_t *session1 = malloc(sizeof(hk_session_t));
//     hk_session_t *session2 = malloc(sizeof(hk_session_t));
//     hk_chr_t *chr1 = malloc(sizeof(hk_chr_t));
//     hk_chr_t *chr2 = malloc(sizeof(hk_chr_t));
//     hk_subscription_store_add_session(chr1, session1);
//     hk_subscription_store_add_session(chr1, session2);
//     hk_subscription_store_add_session(chr2, session2);

//     hk_subscription_store_remove_session(session2);
//     hk_subscription_store_add_session(chr1, session2);
//     hk_subscription_store_add_session(chr2, session2);
//     hk_subscription_store_remove_session(session1);
//     hk_subscription_store_add_session(chr1, session1);
//     hk_subscription_store_remove_session_from_subscription(chr1, session2);
//     hk_subscription_store_add_session(chr1, session2);

//     hk_session_t **sessions = hk_subscription_store_get_sessions(chr1);
//     int session_count = hk_ll_count(sessions);
//     TEST_ASSERT_EQUAL_INT(2, session_count);

//     sessions = hk_subscription_store_get_sessions(chr2);
//     session_count = hk_ll_count(sessions);
//     TEST_ASSERT_EQUAL_INT(1, session_count);

//     HK_LOGI("1");
//     hk_subscription_store_remove_session_from_subscription(chr1, session2);
//     HK_LOGI("2");
//     hk_subscription_store_remove_session_from_subscription(chr1, session2);
//     HK_LOGI("3");
//     hk_subscription_store_remove_session_from_subscription(chr1, session1);
//     HK_LOGI("4");
//     hk_subscription_store_remove_session(session1);
//     HK_LOGI("5");
//     hk_subscription_store_remove_session(session1);
//     HK_LOGI("6");

//     sessions = hk_subscription_store_get_sessions(chr1);
//     session_count = hk_ll_count(sessions);
//     TEST_ASSERT_EQUAL_INT(0, session_count);

//     hk_subscription_store_free();
//     free(session1);
//     free(session2);
//     free(chr1);
//     free(chr2);
// }
