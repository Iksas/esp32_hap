#include "hk_server_handlers.h"

#include "../../utils/hk_logging.h"
#include "../../include/hk_mem.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_pair_setup.h"
#include "../../common/hk_pair_verify.h"
#include "../../common/hk_pairings.h"
#include "hk_chrs.h"
#include "hk_server_transport.h"
#include "hk_session.h"
#include "hk_accessories_serializer.h"

#define HK_SERVER_CONTENT_TLV "application/pairing+tlv8"
#define HK_SERVER_CONTENT_JSON "application/hap+json"

static esp_err_t hk_server_handlers_get_request_content(httpd_req_t *request, hk_mem *content)
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

    return ESP_OK;
}

esp_err_t hk_server_handlers_accessories_get(httpd_req_t *request)
{
    esp_err_t ret = ESP_OK;
    hk_mem *response_content = hk_mem_init();

    RUN_AND_CHECK(ret, hk_accessories_serializer_accessories, response_content);

    RUN_AND_CHECK(ret, httpd_resp_set_type, request, HK_SERVER_CONTENT_JSON);
    RUN_AND_CHECK(ret, httpd_resp_send, request, response_content->ptr, response_content->size);

    hk_mem_free(response_content);

    return ret;
}

esp_err_t hk_server_handlers_characteristics_get(httpd_req_t *request)
{
    esp_err_t ret = ESP_OK;
    hk_mem *response_content = hk_mem_init();
    size_t query_length = httpd_req_get_url_query_len(request) + 1;
    char query[query_length];

    RUN_AND_CHECK(ret, httpd_req_get_url_query_str, request, query, query_length);
    char ids[query_length];
    RUN_AND_CHECK(ret, httpd_query_key_value, query, "id", ids, query_length);

    RUN_AND_CHECK(ret, hk_chrs_get, ids, response_content);

    RUN_AND_CHECK(ret, httpd_resp_set_type, request, HK_SERVER_CONTENT_JSON);
    RUN_AND_CHECK(ret, httpd_resp_send, request, response_content->ptr, response_content->size);

    hk_mem_free(response_content);

    return ret;
}

esp_err_t hk_server_handlers_characteristics_put(httpd_req_t *request)
{
    HK_LOGV("hk_server_handlers_characteristics_put");

    esp_err_t ret = ESP_OK;
    hk_mem *request_content = hk_mem_init();

    RUN_AND_CHECK(ret, hk_server_handlers_get_request_content, request, request_content);
    int socket = httpd_req_to_sockfd(request);
    RUN_AND_CHECK(ret, hk_chrs_put, request_content, request->handle, socket);

    RUN_AND_CHECK(ret, httpd_resp_set_status, request, HTTPD_204);
    RUN_AND_CHECK(ret, httpd_resp_send, request, NULL, 0);

    hk_mem_free(request_content);

    return ret;
}

esp_err_t hk_server_handlers_identify_post(httpd_req_t *request)
{
    HK_LOGV("hk_server_handlers_identify_post");

    esp_err_t ret = ESP_OK;
    
    int socket = httpd_req_to_sockfd(request);

    RUN_AND_CHECK(ret, hk_chrs_identify, socket);
    RUN_AND_CHECK(ret, httpd_resp_set_status, request, HTTPD_204);
    RUN_AND_CHECK(ret, httpd_resp_send, request, NULL, 0);

    return ret;
}

esp_err_t hk_server_handlers_pair_setup_post(httpd_req_t *request)
{
    HK_LOGV("hk_server_handlers_pair_setup_post");

    esp_err_t ret = ESP_OK;
    hk_mem *request_content = hk_mem_init();
    hk_mem *response_content = hk_mem_init();

    int socket = httpd_req_to_sockfd(request);
    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(request->handle, socket);
    
    RUN_AND_CHECK(ret, hk_server_handlers_get_request_content, request, request_content);
    RUN_AND_CHECK(ret, hk_pair_setup, request_content, response_content, session->keys);
    RUN_AND_CHECK(ret, httpd_resp_set_type, request, HK_SERVER_CONTENT_TLV);
    RUN_AND_CHECK(ret, httpd_resp_send, request, response_content->ptr, response_content->size);

    hk_mem_free(request_content);
    hk_mem_free(response_content);

    return ret;
}

esp_err_t hk_server_handlers_pair_verify_post(httpd_req_t *request)
{
    HK_LOGV("hk_server_handlers_pair_verify_post");

    esp_err_t ret = ESP_OK;
    hk_mem *request_content = hk_mem_init();
    hk_mem *response_content = hk_mem_init();
    bool session_is_secure = false;

    int socket = httpd_req_to_sockfd(request);
    RUN_AND_CHECK(ret, hk_server_handlers_get_request_content, request, request_content);

    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(request->handle, socket); // todo: session is set in the method before, move this line when session is initialized eralier.
    RUN_AND_CHECK(ret, hk_pair_verify, request_content, response_content, session->keys, &session_is_secure);

    RUN_AND_CHECK(ret, httpd_resp_set_type, request, HK_SERVER_CONTENT_TLV);
    RUN_AND_CHECK(ret, httpd_resp_send, request, response_content->ptr, response_content->size);

    if (session_is_secure)
    {
        RUN_AND_CHECK(ret, hk_server_transport_set_session_secure, request->handle, socket);
        HK_LOGD("%d - Pairing verified, now communicating encrypted.", session->socket);
    }

    hk_mem_free(request_content);
    hk_mem_free(response_content);

    return ret;
}