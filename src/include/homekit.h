#pragma once

#include "homekit_srvs.h"
#include "homekit_chrs.h"
#include "homekit_categories.h"
#include <stdlib.h>
#include <stdbool.h>

void hk_init(const char *name, const hk_categories_t category, const char *code);
void hk_setup_start();
void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)());
void hk_setup_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden);
void *hk_setup_add_chr(hk_chr_types_t type, void *(*read)(size_t*), void *(*write)(void *, size_t, size_t*), bool can_notify);
void hk_setup_finish();
void hk_reset();

void hk_notify(void *chr);