#pragma once

#include <esp_err.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

#include "../../include/hk_mem.h"
#include "../../common/hk_conn_key_store.h"

#define HK_SESSION_RESPONSE_MESSAGE 0
#define HK_SESSION_RESPONSE_EVENT 1

#define HK_SESSION_CONTENT_TLV 0
#define HK_SESSION_CONTENT_JSON 1

#define HK_SESSION_HTML_METHOD_NOT_SET 0
#define HK_SESSION_HTML_METHOD_GET 1
#define HK_SESSION_HTML_METHOD_POST 3
#define HK_SESSION_HTML_METHOD_PUT 4

struct hk_session;

typedef struct
{
    hk_mem *content;
    hk_mem *query;
    hk_mem *url;
    uint8_t method;
} hk_session_request_t;

typedef struct
{
    QueueHandle_t data_to_send;
    hk_mem *data;
    hk_mem *content;
    size_t result;
    size_t type;
    size_t content_type;
} hk_session_response_t;

typedef struct hk_session
{
    hk_session_request_t *request;
    hk_session_response_t *response;    
    hk_conn_key_store_t *keys;

    int socket;
    bool should_close;
    bool kill; // states that all sessions with the same device id have to be killed
} hk_session_t;

hk_session_t *hk_session_init(int socket);
void hk_session_clean(hk_session_t *session);
void hk_session_clean_response(hk_session_t *session);
void hk_session_free(hk_session_t *session_context);

void hk_session_send(hk_session_t *session);