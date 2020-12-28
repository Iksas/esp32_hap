#pragma once

#include <esp_err.h>
#include <esp_http_server.h>
#include <stdbool.h>

#include "../../common/hk_conn_key_store.h"

#define HK_MAX_RECV_SIZE 1024 // refer to spec 6.5.2
#define HK_AAD_SIZE 2
#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define HK_MAX_DATA_SIZE HK_MAX_RECV_SIZE - HK_AAD_SIZE - HK_AUTHTAG_SIZE

typedef struct hk_server_transport_context
{
    char *received_buffer;
    size_t received_submitted_length;
    size_t received_length;
    size_t received_frame_count;
    size_t sent_frame_count;
    bool is_secure;
    hk_conn_key_store_t *keys;
} hk_server_transport_context_t;

hk_server_transport_context_t *hk_server_transport_context_init();
void hk_server_transport_context_free(void *context);
hk_server_transport_context_t *hk_server_transport_context_get(httpd_handle_t handle, int socket);
