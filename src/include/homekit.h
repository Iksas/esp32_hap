#pragma once

#include "homekit_services.h"
#include "homekit_characteristics.h"
#include "homekit_categories.h"
#include <stdlib.h>
#include <stdbool.h>

void hk_init(const char *name, const hk_categories_t category, const char *code);
void hk_setup_start();
void hk_setup_add_accessory(const char *name, const char *manufacturer, const char *model, const char *serial_number, const char *revision, void (*identify)());
void hk_setup_add_service(hk_service_types_t service_type, bool primary, bool hidden);
void *hk_setup_add_characteristic(hk_characteristic_types_t type, void *(*read)(), void (*write)(void *), bool can_notify);
void hk_setup_finish();
void hk_reset();

void hk_notify(void *characteristic);