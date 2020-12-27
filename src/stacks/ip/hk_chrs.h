#pragma once

#include "../../include/hk_mem.h"
#include "hk_session.h"

#include <stdio.h>
#include <esp_err.h>

esp_err_t hk_chrs_get(char *ids, hk_mem *response);
esp_err_t hk_chrs_put(hk_mem *request, void *http_handle, int socket);
void hk_chrs_identify(hk_session_t *session_context);
void hk_chrs_notify(void *chr);