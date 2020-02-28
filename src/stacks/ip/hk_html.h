#pragma once

#include "../../include/hk_mem.h"

#include "hk_session.h"

#include <esp_err.h>

#define HK_HTML_200 "200 OK"
#define HK_HTML_204 "204 No Content"
#define HK_HTML_207 "207 Multi-Status"
#define HK_HTML_400 "400 Bad Request"
#define HK_HTML_404 "404 Not Found"
#define HK_HTML_422 "422 Unprocessable Entity"
#define HK_HTML_500 "500 Internal Server Error"
#define HK_HTML_503 "503 Service Unavailable"

#define HK_HTML_CONTENT_TLV "application/pairing+tlv8"
#define HK_HTML_CONTENT_JSON "application/hap+json"

#define HK_HTML_PROT_HTTP "HTTP/1.1 "
#define HK_HTML_PROT_EVENT "EVENT/1.0 "

void hk_html_append_response_start(hk_session_t* session, const char* protocol, const char* status);
void hk_html_append_header(hk_session_t *session, const char *key, const char *value);
esp_err_t hk_html_response_send(hk_session_t* session);
esp_err_t hk_html_response_send_empty(hk_session_t *session, const char* status);
