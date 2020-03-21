#include "hk_html.h"

#include "../../utils/hk_logging.h"
#include "hk_com.h"

esp_err_t hk_html_send(hk_session_t *session)
{    
    return hk_com_send_data(session);
}

void hk_html_append_response_start(hk_session_t *session, const char* protocol, const char* status)
{
    hk_mem_append_string(session->response->data, protocol);
    hk_mem_append_string(session->response->data, status);
    hk_mem_append_string(session->response->data, "\r\n");
}

void hk_html_append_header(hk_session_t *session, const char *key, const char *value)
{
    hk_mem_append_string(session->response->data, key);
    hk_mem_append_string(session->response->data, ": ");
    hk_mem_append_string(session->response->data, value);
    hk_mem_append_string(session->response->data, "\r\n");
}

esp_err_t hk_html_response_send(hk_session_t *session)
{
    char length_string[6];
    sprintf(length_string, "%d", session->response->content->size);

    hk_html_append_header(session, "Content-Length", (const char *)length_string);
    hk_mem_append_string(session->response->data, "\r\n");
    hk_mem_append_buffer(session->response->data, session->response->content->ptr, session->response->content->size);

    return hk_html_send(session);
}

esp_err_t hk_html_response_send_empty(hk_session_t *session, const char* status)
{
    hk_mem_append_string(session->response->data, "HTTP/1.1 ");
    hk_mem_append_string(session->response->data, status);
    hk_mem_append_string(session->response->data, "\r\n\r\n");

    return hk_html_send(session);
}