#pragma once

#include <esp_http_server.h>

#include "../../include/hk_mem.h"
#include "../../common/hk_conn_key_store.h"

#include <esp_err.h>

typedef struct hk_server_transport_context
{
    int received_frame_count;
    int sent_frame_count;
    bool is_secure;
} hk_server_transport_context_t;

int hk_server_transport_recv(httpd_handle_t handle, int socket, char *buffer, size_t buffer_length, int flags);

esp_err_t hk_server_transport_encrypt(hk_server_transport_context_t *context, hk_conn_key_store_t *keys, hk_mem *in, 
    esp_err_t (*callback)(hk_mem *frame_data, void *args), void* args);

hk_server_transport_context_t* hk_server_transport_context_init();
void hk_server_transport_context_free(hk_server_transport_context_t* data);