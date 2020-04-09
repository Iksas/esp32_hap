#include "hk_res.h"
#include "hk_tlv.h"
#include "hk_logging.h"
#include "hk_ll.h"

#include <string.h>
#include <stdio.h>

hk_tlv_t *hk_tlv_add_buffer(hk_tlv_t *tlv_list, char type, char *data, size_t size)
{
    size_t size_added = 0;
    do
    {
        size_t size_to_add = size > 255 ? 255 : size;
        size -= size_to_add;

        tlv_list = hk_ll_init(tlv_list);
        tlv_list->type = type;
        tlv_list->length = size_to_add;
        tlv_list->value = malloc(size_to_add);
        memcpy(tlv_list->value, data + size_added, size_to_add);
        size_added += size_to_add;
    } while (size > 0);

    return tlv_list;
}

hk_tlv_t *hk_tlv_add_mem(hk_tlv_t *tlv_list, char type, hk_mem* mem){
    return hk_tlv_add_buffer(tlv_list, type, mem->ptr, mem->size);
}

hk_tlv_t *hk_tlv_add_str(hk_tlv_t *tlv_list, char type, char *data)
{
    return hk_tlv_add_buffer(tlv_list, type, data, strlen(data));
}

hk_tlv_t *hk_tlv_add(hk_tlv_t *tlv_list, char type, hk_mem *data)
{
    return hk_tlv_add_buffer(tlv_list, type, data->ptr, data->size);
}

hk_tlv_t *hk_tlv_add_uint8(hk_tlv_t *tlv_list, char type, uint8_t data)
{
    return hk_tlv_add_buffer(tlv_list, type, (char *)&data, 1);
}

hk_tlv_t *hk_tlv_add_uint16(hk_tlv_t *tlv_list, char type, uint16_t data)
{
    return hk_tlv_add_buffer(tlv_list, type, (char *)&data, 2);
}

esp_err_t hk_tlv_get_mem_by_type(hk_tlv_t *tlv, char type, hk_mem *result)
{
    esp_err_t res = HK_RES_MALFORMED_REQUEST;

    while(tlv) {
        if (tlv->type == type)
        {
            hk_mem_append_buffer(result, tlv->value, tlv->length);
            res = HK_RES_OK;
        }

        tlv = hk_ll_next(tlv);
    }

    if (res != HK_RES_OK)
    {
        HK_LOGE("Error getting tlv for type %d.", type);
    }

    return res;
}

hk_tlv_t *hk_tlv_get_tlv_by_type(hk_tlv_t *tlv, char type)
{
    while (tlv)
    {
        if (tlv->type == type)
        {
            return tlv;
        }

        tlv = hk_ll_next(tlv);
    }

    return NULL;
}

void hk_tlv_serialize(hk_tlv_t *tlv_list, hk_mem *result)
{
    tlv_list = hk_ll_reverse(tlv_list);
    hk_ll_foreach(tlv_list, tlv)
    {
        hk_mem_append_buffer(result, &tlv->type, 1);
        hk_mem_append_buffer(result, &tlv->length, 1);
        hk_mem_append_buffer(result, tlv->value, tlv->length);
    }
    tlv_list = hk_ll_reverse(tlv_list);
}

hk_tlv_t *hk_tlv_deserialize(hk_mem *data)
{
    return hk_tlv_deserialize_buffer(data->ptr, data->size);
}

hk_tlv_t *hk_tlv_deserialize_buffer(char *data, size_t size)
{
    hk_tlv_t *tlv_list = NULL;

    for (size_t i = 0; i < size;)
    {
        char type = data[i++];
        char length = data[i++];

        tlv_list = hk_ll_init(tlv_list);
        tlv_list->type = type;
        tlv_list->length = length;
        tlv_list->value = malloc(tlv_list->length);
        memcpy(tlv_list->value, data + i, length);

        i += length;
    }

    tlv_list = hk_ll_reverse(tlv_list);

    return tlv_list;
}

size_t hk_tlv_get_size(hk_tlv_t *tlv_list)
{
    size_t size = 0;

    hk_ll_foreach(tlv_list, tlv)
    {
        size += 2;
        size += tlv->length;
    }

    return size;
}

void hk_tlv_free(hk_tlv_t *tlv_list)
{
    hk_ll_foreach(tlv_list, tlv)
    {
        free(tlv->value);
        tlv->value = NULL;
    }

    hk_ll_free(tlv_list);
}

void hk_tlv_log(const char *title, hk_tlv_t *tlv_list, bool with_value, bool formatted)
{
    printf("-------------- %s --------------\n", title);
    hk_ll_foreach(tlv_list, tlv)
    {
        printf("type: %x", tlv->type);
        printf(", length: %d, value: ", tlv->length);

        if (with_value)
        {
            for (size_t i = 0; i < tlv->length; i++)
            {
                if (i % 8 == 0 && i != 0 && formatted)
                {
                    printf("\t");
                }

                if (i % 16 == 0 && formatted)
                {
                    printf("\n");
                }
                if(formatted){
                    printf("0x%02x, ", tlv->value[i]);
                }else{
                    printf("%02x", tlv->value[i]);
                }
            }
        }

        printf("\n");
    }

    printf("------------------------------------------\n");
}