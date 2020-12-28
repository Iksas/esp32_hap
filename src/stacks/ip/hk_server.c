#include "hk_server.h"

#include <stdbool.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <esp_http_server.h>

#include "../../utils/hk_logging.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_pair_setup.h"
#include "../../common/hk_pair_verify.h"
#include "../../common/hk_pairings.h"
#include "hk_chrs.h"
#include "hk_server_handlers.h"
#include "hk_server_transport.h"
#include "hk_accessories_serializer.h"

typedef struct
{
    int socket;
    hk_mem *message;
} hk_server_send_args_t;

httpd_handle_t hk_server_handle;

static httpd_uri_t hk_server_accessories_get = {
    .uri = "/accessories",
    .method = HTTP_GET,
    .handler = hk_server_handlers_accessories_get,
    .user_ctx = NULL};

static httpd_uri_t hk_server_characteristics_get = {
    .uri = "/characteristics",
    .method = HTTP_GET,
    .handler = hk_server_handlers_characteristics_get,
    .user_ctx = NULL};

static httpd_uri_t hk_server_characteristics_put = {
    .uri = "/characteristics",
    .method = HTTP_PUT,
    .handler = hk_server_handlers_characteristics_put,
    .user_ctx = NULL};

static httpd_uri_t hk_server_identify_post = {
    .uri = "/identify",
    .method = HTTP_POST,
    .handler = hk_server_handlers_identify_post,
    .user_ctx = NULL};

static httpd_uri_t hk_server_pair_setup_post = {
    .uri = "/pair-setup",
    .method = HTTP_POST,
    .handler = hk_server_handlers_pair_setup_post,
    .user_ctx = NULL};

static httpd_uri_t hk_server_pairings_post = {
    .uri = "/pairings",
    .method = HTTP_POST,
    .handler = hk_server_handlers_pairings_post,
    .user_ctx = NULL};

static httpd_uri_t hk_server_pair_verify_post = {
    .uri = "/pair-verify",
    .method = HTTP_POST,
    .handler = hk_server_handlers_pair_verify_post,
    .user_ctx = NULL};

esp_err_t hk_server_start(void)
{
    esp_err_t ret = ESP_OK;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 5556;
    config.open_fn = hk_server_transport_on_open_connection;

    // Start the httpd server
    HK_LOGD("Starting server on port: '%d'", config.server_port);
    RUN_AND_CHECK(ret, httpd_start, &hk_server_handle, &config);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_accessories_get);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_characteristics_get);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_characteristics_put);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_identify_post);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_pair_setup_post);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_pair_verify_post);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, hk_server_handle, &hk_server_pairings_post);

    if (ret == ESP_OK)
    {
        HK_LOGD("Server started!");
    }

    return ret;
}

static void hk_server_send(void *arg)
{
    hk_server_send_args_t *send_args = (hk_server_send_args_t *)arg;

    esp_err_t ret = hk_server_transport_send_unsolicited(hk_server_handle, send_args->socket, send_args->message);
    if (ret != ESP_OK)
    {
        HK_LOGE("%d - Error sending unsolicited message to client.", send_args->socket);
    }

    hk_mem_free(send_args->message);
    free(send_args);
}

esp_err_t hk_server_send_async(int socket, hk_mem *message)
{
    hk_server_send_args_t *send_args = (hk_server_send_args_t *)malloc(sizeof(hk_server_send_args_t));
    send_args->message = message;
    send_args->socket = socket;

    return httpd_queue_work(hk_server_handle, hk_server_send, send_args);
}
