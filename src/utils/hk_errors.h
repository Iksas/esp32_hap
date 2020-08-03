#pragma once

#include <esp_err.h>

#define ESP_ERR_HK_TERMINATE 0x110001
#define ESP_ERR_HK_UNSUPPORTED_REQUEST 0x110002
#define ESP_ERR_HK_UNKNOWN 0x110003

const char *hk_error_to_name(esp_err_t err);