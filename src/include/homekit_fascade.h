#pragma once

void *hk_setup_add_switch(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    bool primary,
    void (*identify)(),
    void *(*read)(),
    void (*write)(void *));

void *hk_setup_add_motion_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    bool primary,
    void *(*read)());