#pragma once

#include <esp_http_server.h>

#include "../../include/hk_mem.h"
#include "../../common/hk_conn_key_store.h"

#include <esp_err.h>

esp_err_t hk_server_transport_on_open_connection(httpd_handle_t hd, int sockfd);
esp_err_t hk_server_transport_set_session_secure(httpd_handle_t handle, int socket);