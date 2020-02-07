#pragma once

#include "../../include/homekit_characteristics.h"
#include "../../include/homekit_services.h"
#include "../../include/homekit_characteristics.h"
#include "../../common/hk_characteristics_properties.h"

#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    size_t iid;
    size_t aid;
    hk_characteristic_types_t type;
    void *static_value;
    void *(*read)();
    void (*write)(void *);
    bool can_notify;
} hk_characteristic_t;

typedef struct
{
    size_t iid;
    hk_service_types_t type;
    bool primary;
    bool hidden;
    hk_characteristic_t *characteristics;
} hk_service_t;

typedef struct
{
    size_t aid;
    hk_service_t *services;
} hk_accessory_t;

void hk_accessories_store_add_accessory();
void hk_accessories_store_add_service(hk_service_types_t service_type, bool primary, bool hidden);
void* hk_accessories_store_add_characteristic(hk_characteristic_types_t characteristic_type, void *(*read)(), void (*write)(void *), bool can_notify);
void hk_accessories_store_add_characteristic_static_read(hk_characteristic_types_t type, void *value);
void hk_accessories_store_end_config();

hk_accessory_t *hk_accessories_store_get_accessories();
hk_characteristic_t *hk_accessories_store_get_characteristic(size_t aid, size_t iid);
hk_characteristic_t *hk_accessories_store_get_identify_characteristic();