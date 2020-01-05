#pragma once

#include "hk_mem.h"
#include <stdlib.h>
#include <stdbool.h>
#include <esp_err.h>

size_t hk_store_init();

void hk_store_free();

bool hk_store_keys_can_get();
void hk_store_key_priv_get();
void hk_store_key_priv_set(hk_mem *value);
void hk_store_key_pub_get(hk_mem *value);
void hk_store_key_pub_set(hk_mem *value);

bool hk_store_is_paired_get();
void hk_store_is_paired_set(bool paired);

const char *hk_store_code_get();
void hk_store_code_set(const char* code);

void hk_store_pairings_get(hk_mem *pairings);
void hk_store_pairings_set(hk_mem *pairings);
