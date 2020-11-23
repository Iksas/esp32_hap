#include "hk_html_parser.h"

#include <http_parser.h>

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"

static esp_err_t hk_html_url(http_parser *parser, const char *url, size_t length)
{
    hk_session_t *session = parser->data;
    const char *query = memchr(url, '?', length);

    size_t url_length = query ? query - url : length;
    hk_mem_append_buffer(session->request->url, (char *)url, url_length);
    hk_mem_append_string_terminator(session->request->url);

    if (query)
    {
        size_t query_length = length - url_length - 1;
        hk_mem_append_buffer(session->request->query, (char *)query + 1, query_length);
        hk_mem_append_string_terminator(session->request->query);
    }

    return ESP_OK;
}

static esp_err_t hk_html_header_field(http_parser *parser, const char *at, size_t length)
{
    return ESP_OK;
}

static esp_err_t hk_html_header_value(http_parser *parser, const char *at, size_t length)
{
    return ESP_OK;
}

static esp_err_t hk_html_headers_complete(http_parser *parser)
{
    //HK_LOGD("Headers complete.");
    return ESP_OK;
}

static esp_err_t hk_html_on_body(http_parser *parser, const char *body, size_t length)
{
    hk_session_t *session = parser->data;
    hk_mem_append_buffer(session->request->content, (char *)body, length);
    hk_mem_append_string_terminator(session->request->content);

    return ESP_OK;
}

static esp_err_t hk_html_message_complete(http_parser *parser)
{
    return ESP_OK;
}

esp_err_t hk_html_parser_parse(hk_mem *message, hk_session_t *session)
{
    // setup parser and data
    http_parser parser;
    http_parser_settings parser_settings;
    http_parser_init(&parser, HTTP_REQUEST);
    http_parser_settings_init(&parser_settings);

    parser_settings.on_url = hk_html_url;
    parser_settings.on_header_field = hk_html_header_field;
    parser_settings.on_header_value = hk_html_header_value;
    parser_settings.on_headers_complete = hk_html_headers_complete;
    parser_settings.on_body = hk_html_on_body;
    parser_settings.on_message_complete = hk_html_message_complete;

    parser.data = session;

    // parse
    http_parser_execute(&parser, &parser_settings, message->ptr, message->size);

    session->request->method = parser.method;

    return ESP_OK;
}

esp_err_t hk_html_parser_get_query_value(hk_mem *query, const char *key, hk_mem *value)
{
    char *q = query->ptr;

    char *v = strchr(q, '='); //get first value
    while (v != NULL)         // as long we have a value, go on
    {
        v++; // remove = from value

        if (strncmp(key, q, v - q - 1) == 0) // check if key equals given
        {
            char *v_end = strchr(v, '&'); // in order to get value length, we look for next key
            size_t v_length = 0;
            if (v_end == NULL)
            {
                v_length = strlen(v); // if no next key was found, value string is terminated. Get length.
            }
            else
            {
                v_length = v_end - v; // else get length by pointer arithmetic
            }

            hk_mem_append_buffer(value, v, v_length); // append value
            hk_mem_append_string_terminator(value);   // append null terminator
            return ESP_OK;
        }

        q = strchr(q, '&'); // find next key value pair
        if (q != NULL)      // if found
        {
            q++;                // remove &
            v = strchr(q, '='); //get next value
        }
    }

    return ESP_FAIL;
}