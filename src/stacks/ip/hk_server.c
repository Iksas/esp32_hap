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
#include "hk_com.h"
#include "hk_session.h"
#include "hk_accessories_serializer.h"
#include "hk_advertising.h"

void hk_server_handle(hk_session_t *session)
{
    HK_LOGD("%d - Handling %s, %d", session->socket, session->request->url->ptr, session->request->method);

    if (hk_mem_equal_str(session->request->url, "/pair-setup") && HK_SESSION_HTML_METHOD_POST == session->request->method)
    {
        hk_mem *device_id = hk_mem_init();
        bool is_paired = false;
        session->response->result = hk_pair_setup(session->request->content, session->response->content, session->keys, device_id, &is_paired);
        if (device_id->size > 0)
        {
            session->device_id = strndup(device_id->ptr, device_id->size);
        }
        if (is_paired)
        {
            hk_advertising_update_paired();
        }

        hk_session_send(session);
        hk_mem_free(device_id);
    }
    else if (hk_mem_equal_str(session->request->url, "/pair-verify") && HK_SESSION_HTML_METHOD_POST == session->request->method)
    {
        bool session_is_secure = false;
        session->response->result = hk_pair_verify(session->request->content, session->response->content, session->keys, &session_is_secure);

        hk_session_send(session);

        if (session_is_secure)
        {
            session->is_secure = true;
            HK_LOGD("%d - Pairing verified, now communicating encrypted.", session->socket);
        }
    }
    else if (hk_mem_equal_str(session->request->url, "/accessories") && HK_SESSION_HTML_METHOD_GET == session->request->method)
    {
        hk_accessories_serializer_accessories(session->response->content);
        hk_log_print_as_string("accs", session->response->content->ptr, session->response->content->size);
        session->response->content_type = HK_SESSION_CONTENT_JSON;
        HK_LOGD("%d - Returning accessories.", session->socket);
        hk_session_send(session);
    }
    else if (hk_mem_equal_str(session->request->url, "/characteristics") && HK_SESSION_HTML_METHOD_GET == session->request->method)
    {
        hk_chrs_get(session);
    }
    else if (hk_mem_equal_str(session->request->url, "/characteristics") && HK_SESSION_HTML_METHOD_PUT == session->request->method)
    {
        hk_chrs_put(session);
    }
    else if (hk_mem_equal_str(session->request->url, "/pairings") && HK_SESSION_HTML_METHOD_POST == session->request->method)
    {
        bool is_paired = true;
        session->response->result = hk_pairings(session->request->content, session->response->data, &session->kill, &is_paired);
        hk_session_send(session);
        if (!is_paired)
        {
            hk_advertising_update_paired();
        }
    }
    else if (hk_mem_equal_str(session->request->url, "/identify") && HK_SESSION_HTML_METHOD_POST == session->request->method)
    {
        hk_chrs_identify(session);
    }
    else
    {
        HK_LOGE("Could not find handler for '%s' and method %d.", session->request->url->ptr, session->request->method);
        hk_html_response_send_empty(session, HK_HTML_404);
        session->should_close = true;
    }
}

esp_err_t hk_server_receiver(hk_session_t *session, hk_mem *data)
{
    hk_session_clean(session);
    hk_mem *response = hk_mem_init();

    // html parses the content of request data and calls the configured handler
    esp_err_t ret = hk_html_parser_parse(data, session);
    if (ret != ESP_OK)
    {
        HK_LOGE("Could not process received data of socket %d.", session->socket);
    }

    hk_server_handle(session);

    hk_mem_free(response);

    return ret;
}

void hk_server_start(void)
{
    hk_com_start(5556, hk_server_receiver);
}