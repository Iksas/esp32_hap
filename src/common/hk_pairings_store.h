#pragma once

#include "../include/hk_mem.h"

#include <esp_err.h>
#include <stdbool.h>

esp_err_t hk_pairings_store_add(hk_mem* device_id, hk_mem* device_ltpk, bool is_admin);
esp_err_t hk_pairings_store_remove(hk_mem *device_id);
esp_err_t hk_pairings_store_device_exists(hk_mem *device_id, bool *exists);
esp_err_t hk_pairings_store_is_admin(hk_mem *device_id, bool *is_admin);
esp_err_t hk_pairings_store_has_admin_pairing(bool *is_admin);
esp_err_t hk_pairings_store_has_pairing(bool *is_admin);
esp_err_t hk_pairings_store_remove_all();
esp_err_t hk_pairings_store_list();
esp_err_t hk_pairings_store_ltpk_get(hk_mem *device_id, hk_mem *device_ltpk);

esp_err_t hk_pairings_log_devices();
