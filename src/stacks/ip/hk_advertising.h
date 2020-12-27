#pragma once

#include <stdlib.h>
#include <stdbool.h>
#include <esp_err.h>

void hk_advertising_init(const char *name, size_t category, size_t config_version);
esp_err_t hk_advertising_update_paired();
esp_err_t hk_advertising_global_state_next();
esp_err_t hk_advertising_reset();
