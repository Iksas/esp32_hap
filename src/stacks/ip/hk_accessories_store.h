#pragma once

#include "../../include/hk_chrs.h"
#include "../../include/hk_srvs.h"
#include "../../include/hk_chrs.h"
#include "../../include/hk_mem.h"
#include "../../common/hk_chrs_properties.h"

#include <stdlib.h>
#include <stdbool.h>

typedef struct
{
    size_t iid;
    size_t aid;
    hk_chr_types_t type;
    void *static_value;
    esp_err_t (*read)(hk_mem* response);
    esp_err_t (*write)(hk_mem* request);
    bool can_notify;
} hk_chr_t;

typedef struct
{
    size_t iid;
    hk_srv_types_t type;
    bool primary;
    bool hidden;
    hk_chr_t *chrs;
} hk_srv_t;

typedef struct
{
    size_t aid;
    hk_srv_t *srvs;
} hk_accessory_t;

void hk_accessories_store_add_accessory();
void hk_accessories_store_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden);
esp_err_t hk_accessories_store_add_chr(hk_chr_types_t chr_type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request), bool can_notify, void **chr_ptr);
void hk_accessories_store_add_chr_static_read(hk_chr_types_t type, void *value);
void hk_accessories_store_end_config();

hk_accessory_t *hk_accessories_store_get_accessories();
hk_chr_t *hk_accessories_store_get_chr(size_t aid, size_t iid);
hk_chr_t *hk_accessories_store_get_identify_chr();
void hk_accessories_free();