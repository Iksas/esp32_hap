#pragma once

#include <stdlib.h>
#include <esp_types.h>
#include <esp_err.h>

#include "../include/hk_mem.h"

typedef struct
{
    char type;
    char length;
    char *value;
} hk_tlv_t;

hk_tlv_t *hk_tlv_add_str(hk_tlv_t *tlv_list, char type, char *data);
hk_tlv_t *hk_tlv_add_buffer(hk_tlv_t *tlv_list, char type, char *data, size_t length);
hk_tlv_t *hk_tlv_add_mem(hk_tlv_t *tlv_list, char type, hk_mem* mem);
hk_tlv_t *hk_tlv_add_uint8(hk_tlv_t *tlv_list, char type, uint8_t data);
hk_tlv_t *hk_tlv_add_uint16(hk_tlv_t *tlv_list, char type, uint16_t data);
hk_tlv_t *hk_tlv_add(hk_tlv_t *tlv_list, char type, hk_mem *data);
esp_err_t hk_tlv_get_mem_by_type(hk_tlv_t *tlv, char type, hk_mem *result);
hk_tlv_t *hk_tlv_get_tlv_by_type(hk_tlv_t *tlv_list, char type);
size_t hk_tlv_get_size(hk_tlv_t *tlv_list);
void hk_tlv_free(hk_tlv_t *tlv_list);

void hk_tlv_serialize(hk_tlv_t *tlv_list, hk_mem *result);
hk_tlv_t *hk_tlv_deserialize(hk_mem *data);
hk_tlv_t *hk_tlv_deserialize_buffer(char *data, size_t size);

void hk_tlv_log(const char *title, hk_tlv_t *tlv_list, bool with_value, bool formatted);
