#pragma once

void *hk_setup_add_switch(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    bool primary,
    void (*identify)(),
    void *(*read)(size_t*),
    void *(*write)(void *, size_t, size_t*));

void *hk_setup_add_motion_sensor(
    const char *name,
    const char *manufacturer,
    const char *model,
    const char *serial_number,
    const char *revision,
    bool primary,
    void *(*read)(size_t*));