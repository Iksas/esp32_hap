#pragma once

#include "../utils/hk_mem.h"

#include <esp_err.h>
#include <stdbool.h>

void hk_pairings_store_add(hk_mem* device_id, hk_mem* device_ltpk, bool is_admin);
void hk_pairings_store_remove(hk_mem *device_id);
bool hk_pairings_store_is_admin(hk_mem *device_id);
bool hk_pairings_store_has_admin_pairing();
void hk_pairings_store_remove_all();
esp_err_t hk_pairings_store_list();
esp_err_t hk_pairings_store_ltpk_get(hk_mem *device_id, hk_mem *device_ltpk);

void hk_pairings_log_devices();
