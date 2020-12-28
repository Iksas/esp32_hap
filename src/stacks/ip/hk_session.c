#include "hk_session.h"

#include "../../utils/hk_logging.h"

hk_session_t *hk_session_init(int socket)
{
    hk_session_t *session = malloc(sizeof(hk_session_t));
    session->socket = socket;
    session->should_close = false;
    session->kill = false;

    session->request = (hk_session_request_t *)malloc(sizeof(hk_session_request_t));
    session->request->url = hk_mem_init();
    session->request->query = hk_mem_init();
    session->request->content = hk_mem_init();
    session->request->method = HK_SESSION_HTML_METHOD_NOT_SET;

    session->response = (hk_session_response_t *)malloc(sizeof(hk_session_response_t));
    session->response->data_to_send = xQueueCreate(10, sizeof(hk_mem *));
    session->response->data = hk_mem_init();
    session->response->content = hk_mem_init();
    session->response->result = ESP_OK;
    session->response->type = HK_SESSION_RESPONSE_MESSAGE;
    session->response->content_type = HK_SESSION_CONTENT_TLV;

    session->keys = hk_conn_key_store_init();

    return session;
}

void hk_session_clean_response(hk_session_t *session)
{
    hk_mem_set(session->response->data, 0);
    hk_mem_set(session->response->content, 0);
    session->response->result = ESP_OK;
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
    
    free(session);
}
