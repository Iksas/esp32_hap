#pragma once

#include <esp_err.h>
#include "../../include/hk_mem.h"

esp_err_t hk_server_start(void);
esp_err_t hk_server_send_async(int socket, hk_mem *message);
