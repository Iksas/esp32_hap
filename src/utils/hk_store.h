#pragma once

#include "hk_mem.h"
#include <stdlib.h>
#include <stdbool.h>
#include <esp_err.h>

// max size for keys is 15
#define HK_REVISION_STORE_KEY "hk_rvsn"
#define HK_CONFIGURATION_STORE_KEY "hk_cnfgrtn_cnt"

esp_err_t hk_store_init();
esp_err_t hk_store_u8_get(const char *key, uint8_t *value);
esp_err_t hk_store_u8_set(const char *key, uint8_t value);
esp_err_t hk_store_u16_get(const char *key, uint16_t *value);
esp_err_t hk_store_u16_set(const char *key, uint16_t value);
esp_err_t hk_store_blob_get(const char *key, hk_mem *value);
esp_err_t hk_store_blob_set(const char *key, hk_mem *value);
esp_err_t hk_store_erase(const char *key);

void hk_store_free();

bool hk_store_keys_can_get();
esp_err_t hk_store_key_priv_get();
esp_err_t hk_store_key_priv_set(hk_mem *value);
esp_err_t hk_store_key_pub_get(hk_mem *value);
esp_err_t hk_store_key_pub_set(hk_mem *value);

const char *hk_store_code_get();
void hk_store_code_set(const char* code);
