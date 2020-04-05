#pragma once

#include "hk_srvs.h"
#include "hk_chrs.h"
#include "hk_categories.h"
#include "hk_mem.h"
#include <stdlib.h>
#include <stdbool.h>
#include "esp_err.h"

void hk_init(const char *name, const hk_categories_t category, const char *code);
void hk_setup_start();
void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)());
void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden);
void *hk_setup_add_chr(hk_chr_types_t type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request), bool can_notify);
void hk_setup_finish();
void hk_reset();

void hk_notify(void *chr);