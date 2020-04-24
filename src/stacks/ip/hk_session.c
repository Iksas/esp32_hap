#include "hk_session.h"

#include "../../utils/hk_res.h"
#include "../../utils/hk_logging.h"

#include "hk_html.h"

void hk_session_init(hk_session_t *session, int socket)
{
    session->socket = socket;
    session->should_close = false;
    session->device_id = NULL;
    session->kill = false;
    session->is_secure = false;

    session->request = (hk_session_request_t *)malloc(sizeof(hk_session_request_t));
    session->request->url = hk_mem_init();
    session->request->query = hk_mem_init();
    session->request->content = hk_mem_init();
    session->request->method = HK_SESSION_HTML_METHOD_NOT_SET;

    session->response = (hk_session_response_t *)malloc(sizeof(hk_session_response_t));
    session->response->data_to_send = xQueueCreate(10, sizeof(hk_mem *));
    session->response->data = hk_mem_init();
    session->response->content = hk_mem_init();
    session->response->result = HK_RES_OK;
    session->response->type = HK_SESSION_RESPONSE_MESSAGE;
    session->response->content_type = HK_SESSION_CONTENT_TLV;

    session->keys = hk_conn_key_store_init();
    session->encryption_data = hk_encryption_data_init();
}

void hk_session_clean_response(hk_session_t *session)
{
    hk_mem_set(session->response->data, 0);
    hk_mem_set(session->response->content, 0);
    session->response->result = HK_RES_OK;
    session->response->type = HK_SESSION_RESPONSE_MESSAGE;
}

void hk_session_clean(hk_session_t *session)
{
    hk_mem_set(session->request->url, 0);
    hk_mem_set(session->request->query, 0);
    hk_mem_set(session->request->content, 0);
    session->request->method = HK_SESSION_HTML_METHOD_NOT_SET;

    hk_session_clean_response(session);
}

void hk_session_free(hk_session_t *session)
{
    hk_mem_free(session->request->url);
    hk_mem_free(session->request->query);
    hk_mem_free(session->request->content);
    if (session->device_id != NULL)
    {
        free(session->device_id);
    }
    free(session->request);

    hk_mem_free(session->response->data);
    hk_mem_free(session->response->content);

    hk_mem *data_to_send;
    while (xQueueReceive(session->response->data_to_send, &data_to_send, 0))
    {
        hk_mem_free(data_to_send);
    }
    vQueueDelete(session->response->data_to_send);
    free(session->response);
    hk_conn_key_store_free(session->keys);
    hk_encryption_data_free(session->encryption_data);
}

const char *hk_session_get_status(hk_session_t *session)
{
    switch (session->response->result)
    {
    case HK_RES_OK:
        return HK_HTML_200;
    case HK_RES_MALFORMED_REQUEST:
        return HK_HTML_400;
    case HK_RES_UNKNOWN:
        return HK_HTML_500;
    default:
        HK_LOGE("Unknown response status: %d", session->response->result);
        return "";
    }
}

const char *hk_session_get_protocol(hk_session_t *session)
{
    switch (session->response->type)
    {
    case HK_SESSION_RESPONSE_MESSAGE:
        return HK_HTML_PROT_HTTP;
    case HK_SESSION_RESPONSE_EVENT:
        return HK_HTML_PROT_EVENT;
    default:
        HK_LOGE("Unknown response type: %d.", session->response->type);
        return "";
    }
}

const char *hk_session_get_content_type(hk_session_t *session)
{
    switch (session->response->content_type)
    {
    case HK_SESSION_CONTENT_TLV:
        return HK_HTML_CONTENT_TLV;
    case HK_SESSION_CONTENT_JSON:
        return HK_HTML_CONTENT_JSON;
    default:
        HK_LOGE("Unknown response content type: %d", session->response->content_type);
        return "";
    }
}

// void hk_session_set_device_id(hk_session_t *session, hk_mem *device_id)
// {
//
// }

void hk_session_send(hk_session_t *session)
{
    const char *status = hk_session_get_status(session);

    if (session->response->content->size > 0)
    {
        const char *protocol = hk_session_get_protocol(session);
        const char *content = hk_session_get_content_type(session);

        hk_html_append_response_start(session, protocol, status);
        hk_html_append_header(session, "Content-Type", content);
        hk_html_response_send(session);
    }
    else
    {
        hk_html_response_send_empty(session, status);
    }
}