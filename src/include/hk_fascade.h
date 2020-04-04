#pragma once

#include "hk_mem.h"
#include "esp_err.h"

void *hk_setup_add_switch(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    bool primary,
    void (*identify)(),
    esp_err_t (*read)(hk_mem* response),
    esp_err_t (*write)(hk_mem* request, hk_mem* response));

void *hk_setup_add_motion_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    bool primary,
    esp_err_t (*read)(hk_mem* response));