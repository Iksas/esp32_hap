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
#include "hk_server_transport.h"
#include "hk_session.h"
#include "hk_accessories_serializer.h"
#include "hk_advertising.h"

#define HK_SERVER_CONTENT_TLV "application/pairing+tlv8"
#define HK_SERVER_CONTENT_JSON "application/hap+json"

static void hk_server_on_free_session_ctx(void *ctx)
{
    hk_session_t *session = (hk_session_t *)ctx;
    hk_session_free(session);
}

static void hk_server_on_free_session_transport_ctx(void *ctx)
{
    HK_LOGD("Freeing transport ctx.");
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)ctx;
    hk_server_transport_context_free(transport_context);
}

static esp_err_t hk_server_get_request_content(httpd_req_t *request, hk_mem *content)
{
    size_t receive_size = request->content_len;
    hk_mem_set(content, receive_size);
    int ret = httpd_req_recv(request, content->ptr, receive_size);
    if (ret <= 0)
    {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT)
        {
            HK_LOGW("Error getting content of request: HTTPD_SOCK_ERR_TIMEOUT");
            /* In case of timeout one can choose to retry calling
             * httpd_req_recv(), but to keep it simple, here we
             * respond with an HTTP 408 (Request Timeout) error */
            httpd_resp_send_408(request);
        }
        else
        {
            HK_LOGE("Error getting content of request: %d", ret);
            return ESP_FAIL;
        }
    }

    int socket = httpd_req_to_sockfd(request);
    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(request->handle, socket);
    if (session == NULL)
    {
        // todo: move to open connection function
        HK_LOGD("Initializing new session ctx.");
        session = hk_session_init(socket);
        httpd_sess_set_ctx(request->handle, socket, (void *)session, hk_server_on_free_session_ctx);
        // todo: move
        httpd_sess_set_recv_override(request->handle, socket, hk_server_transport_recv);
    }

    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(request->handle, socket);
    if (transport_context == NULL)
    {
        // todo: move to open connection function
        transport_context = hk_server_transport_context_init(socket);
        httpd_sess_set_transport_ctx(request->handle, socket, (void *)transport_context, hk_server_on_free_session_transport_ctx);
    }

    return ESP_OK;
}

static esp_err_t hk_server_pair_verify_handler(httpd_req_t *request)
{
    HK_LOGI("hk_server_pair_verify");

    esp_err_t ret = ESP_OK;
    hk_mem *request_content = hk_mem_init();
    hk_mem *response_content = hk_mem_init();
    bool session_is_secure = false;

    int socket = httpd_req_to_sockfd(request);
    RUN_AND_CHECK(ret, hk_server_get_request_content, request, request_content);

    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(request->handle, socket); // todo: session is set in the method before, move this line when session is initialized eralier.
    RUN_AND_CHECK(ret, hk_pair_verify, request_content, response_content, session->keys, &session_is_secure);

    RUN_AND_CHECK(ret, httpd_resp_set_type, request, HK_SERVER_CONTENT_TLV);
    RUN_AND_CHECK(ret, httpd_resp_send, request, response_content->ptr, response_content->size);
    // hk_session_send(session);
    //->
    // const char *status = hk_session_get_status(session);

    // if (session->response->content->size > 0)
    // {
    //     const char *protocol = hk_session_get_protocol(session);
    //     const char *content = hk_session_get_content_type(session);

    //     hk_html_append_response_start(session, protocol, status);
    //     hk_html_append_header(session, "Content-Type", content);
    //     hk_html_response_send(session);
    // }
    // else
    // {
    //     hk_html_response_send_empty(session, status);
    // }

    if (session_is_secure)
    {
        hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(request->handle, socket);
        transport_context->is_secure = true;
        HK_LOGD("%d - Pairing verified, now communicating encrypted.", session->socket);
    }

    hk_mem_free(request_content);
    hk_mem_free(response_content);

    return ret;
}

/* Maintain a variable which stores the number of times
 * the "/adder" URI has been visited */
// todo: is this needed?
static unsigned visitors = 0;

static httpd_uri_t hk_server_post_pair_verify_definition = {
    .uri = "/pair-verify",
    .method = HTTP_POST,
    .handler = hk_server_pair_verify_handler,
    .user_ctx = &visitors};

esp_err_t hk_server_start(void)
{
    esp_err_t ret = ESP_OK;

    httpd_handle_t server_handle;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 5556;

    // Start the httpd server
    HK_LOGD("Starting server on port: '%d'", config.server_port);
    RUN_AND_CHECK(ret, httpd_start, &server_handle, &config);
    RUN_AND_CHECK(ret, httpd_register_uri_handler, server_handle, &hk_server_post_pair_verify_definition);

    if (ret == ESP_OK)
    {
        HK_LOGD("Server started!");
    }

    return ret;
}


// void hk_server_handle(hk_session_t *session)
// {
//     char *content = hk_mem_to_string(session->request->content);
//     char *url = hk_mem_to_string(session->request->url);
//     char *method;
//     switch (session->request->method)
//     {
//     case HK_SESSION_HTML_METHOD_GET:
//         method = "GET";
//         break;
//     case HK_SESSION_HTML_METHOD_POST:
//         method = "POST";
//         break;
//     case HK_SESSION_HTML_METHOD_PUT:
//         method = "PUT";
//         break;
//     default:
//         method = "UNDEFINED";
//         break;
//     }

//     HK_LOGD("%d - Handling %s: %s", session->socket, method, url);
//     HK_LOGD("Content:\n%s", content);

//     free(content);
//     free(url);

//     if (hk_mem_equal_str(session->request->url, "/pair-setup") && HK_SESSION_HTML_METHOD_POST == session->request->method)
//     {
//         hk_mem *device_id = hk_mem_init();
//         bool is_paired = false;
//         session->response->result = hk_pair_setup(session->request->content, session->response->content, session->keys, device_id, &is_paired);
//         if (device_id->size > 0)
//         {
//             session->device_id = strndup(device_id->ptr, device_id->size);
//         }
//         if (is_paired)
//         {
//             hk_advertising_update_paired();
//         }

//         hk_session_send(session);
//         hk_mem_free(device_id);
//     }
//     else if (hk_mem_equal_str(session->request->url, "/pair-verify") && HK_SESSION_HTML_METHOD_POST == session->request->method)
//     {
//         bool session_is_secure = false;
//         session->response->result = hk_pair_verify(session->request->content, session->response->content, session->keys, &session_is_secure);

//         hk_session_send(session);

//         if (session_is_secure)
//         {
//             session->is_secure = true;
//             HK_LOGD("%d - Pairing verified, now communicating encrypted.", session->socket);
//         }
//     }
//     else if (hk_mem_equal_str(session->request->url, "/accessories") && HK_SESSION_HTML_METHOD_GET == session->request->method)
//     {
//         hk_accessories_serializer_accessories(session->response->content);
//         session->response->content_type = HK_SESSION_CONTENT_JSON;
//         HK_LOGD("%d - Returning accessories.", session->socket);

//         hk_session_send(session);
//     }
//     else if (hk_mem_equal_str(session->request->url, "/characteristics") && HK_SESSION_HTML_METHOD_GET == session->request->method)
//     {
//         hk_chrs_get(session);
//     }
//     else if (hk_mem_equal_str(session->request->url, "/characteristics") && HK_SESSION_HTML_METHOD_PUT == session->request->method)
//     {
//         hk_chrs_put(session);
//     }
//     else if (hk_mem_equal_str(session->request->url, "/pairings") && HK_SESSION_HTML_METHOD_POST == session->request->method)
//     {
//         bool is_paired = true;
//         session->response->result = hk_pairings(session->request->content, session->response->data, &session->kill, &is_paired);
//         hk_session_send(session);
//         if (!is_paired)
//         {
//             hk_advertising_update_paired();
//         }
//     }
//     else if (hk_mem_equal_str(session->request->url, "/identify") && HK_SESSION_HTML_METHOD_POST == session->request->method)
//     {
//         hk_chrs_identify(session);
//     }
//     else
//     {
//         HK_LOGE("Could not find handler for '%s' and method %d.", session->request->url->ptr, session->request->method);
//         hk_html_response_send_empty(session, HK_HTML_404);
//         session->should_close = true;
//     }
// }

// esp_err_t hk_server_receiver(hk_session_t *session, hk_mem *data)
// {
//     hk_session_clean(session);
//     hk_mem *response = hk_mem_init();

//     // html parses the content of request data and calls the configured handler
//     esp_err_t ret = hk_html_parser_parse(data, session);
//     if (ret != ESP_OK)
//     {
//         HK_LOGE("Could not process received data of socket %d.", session->socket);
//     }

//     hk_server_handle(session);

//     hk_mem_free(response);

//     return ret;
// }
