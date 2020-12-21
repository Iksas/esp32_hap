#pragma once

#include <esp_err.h>
#include <esp_http_server.h>

esp_err_t hk_server_handlers_accessories_get(httpd_req_t *request);
esp_err_t hk_server_handlers_characteristics_get(httpd_req_t *request);
esp_err_t hk_server_handlers_pair_verify_post(httpd_req_t *request);