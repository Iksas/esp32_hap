#include "hk_server.h"

#include <stdbool.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <esp_http_server.h>

#include "../../utils/hk_logging.h"
#include "../../include/hk_mem.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_pair_setup.h"
#include "../../common/hk_pair_verify.h"
#include "../../common/hk_pairings.h"
#include "hk_chrs.h"
#include "hk_html.h"
#include "hk_html_parser.h"
#include "hk_server_handlers.h"
#include "hk_server_transport.h"
#include "hk_session.h"
#include "hk_accessories_serializer.h"

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

static httpd_uri_t hk_server_pair_setup_post = {
    .uri = "/pair-setup",
    .method = HTTP_POST,
    .handler = hk_server_handlers_pair_setup_post,
    .user_ctx = NULL};

static httpd_uri_t hk_server_pair_verify_post = {
    .uri = "/pair-verify",
    .method = HTTP_POST,
    .handler = hk_server_handlers_pair_verify_post,
    .user_ctx = NULL};

esp_err_t hk_server_start(void)
{
    esp_err_t ret = ESP_OK;

    httpd_handle_t server_handle;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 5556;
    config.open_fn = hk_server_transport_on_open_connection;

    // Start the httpd server
    HK_LOGD("Starting server on port: '%d'", config.server_port);
    RUN_AND_CHECK(ret, httpd_start, &server_handle, &config);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, server_handle, &hk_server_accessories_get);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, server_handle, &hk_server_characteristics_get);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, server_handle, &hk_server_characteristics_put);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, server_handle, &hk_server_pair_setup_post);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, server_handle, &hk_server_pair_verify_post);

    if (ret == ESP_OK)
    {
        HK_LOGD("Server started!");
    }

    return ret;
}
