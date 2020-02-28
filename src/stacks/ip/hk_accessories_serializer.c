#include "hk_accessories_serializer.h"

#include "../../include/homekit_srvs.h"
#include "../../include/homekit_chrs.h"
#include "../../common/hk_chrs_properties.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_ll.h"

#define HAP_UUID "%08X-0000-1000-8000-0026BB765291"

cJSON *hk_accessories_serializer_format_value(hk_format_t format, void *value)
{
    switch (format)
    {
    case HK_FORMAT_BOOL:
        return cJSON_CreateBool(*(bool *)value);
    case HK_FORMAT_UINT8:
    case HK_FORMAT_UINT32:
    case HK_FORMAT_UINT64:
    case HK_FORMAT_INT:
        return cJSON_CreateNumber(*(int *)value);
    case HK_FORMAT_FLOAT:
        return cJSON_CreateNumber(*(double *)value);
    case HK_FORMAT_STRING:
        return cJSON_CreateString((char *)value);
    case HK_FORMAT_TLV8:
        HK_LOGE("Formatting for tlv values is not implemented for now.");
        return cJSON_CreateNull();
    case HK_FORMAT_DATA:
        HK_LOGE("Formatting for data values is not implemented for now.");
        return cJSON_CreateNull();
    default:
        HK_LOGE("Unknown format found: %d", format);
        return cJSON_CreateNull();
    }
}

void hk_accessories_serializer_value(hk_chr_t *chr, cJSON *j_chr)
{
    hk_format_t format = hk_chrs_properties_get_type(chr->type);
    if (chr->read != NULL)
    {
        void *value = chr->read();
        cJSON_AddItemToObject(j_chr, "value", hk_accessories_serializer_format_value(format, value));
    }
    else if (chr->static_value != NULL)
    {
        cJSON_AddItemToObject(j_chr, "value", hk_accessories_serializer_format_value(format, chr->static_value));
    }
    else
    {
        if (chr->type != HK_CHR_IDENTIFY)
        {
            cJSON_AddNullToObject(j_chr, "value");
        }
    }
}

void hk_accessories_serializer_format(hk_chr_t *chr, cJSON *j_chr)
{
    hk_format_t format = hk_chrs_properties_get_type(chr->type);
    switch (format)
    {
    case HK_FORMAT_BOOL:
        cJSON_AddStringToObject(j_chr, "format", "bool");
        break;
    case HK_FORMAT_UINT8:
        cJSON_AddStringToObject(j_chr, "format", "uint8");
        break;
    case HK_FORMAT_UINT32:
        cJSON_AddStringToObject(j_chr, "format", "uint32");
        break;
    case HK_FORMAT_UINT64:
        cJSON_AddStringToObject(j_chr, "format", "uint64");
        break;
    case HK_FORMAT_INT:
        cJSON_AddStringToObject(j_chr, "format", "int");
        break;
    case HK_FORMAT_FLOAT:
        cJSON_AddStringToObject(j_chr, "format", "float");
        break;
    case HK_FORMAT_STRING:
        cJSON_AddStringToObject(j_chr, "format", "string");
        break;
    case HK_FORMAT_TLV8:
        cJSON_AddStringToObject(j_chr, "format", "tlv8");
        break;
    case HK_FORMAT_DATA:
        cJSON_AddStringToObject(j_chr, "format", "data");
        break;
    default:
        HK_LOGE("Unknown format found: %d", format);
    }
}

void hk_accessories_serializer_perms(hk_chr_t *chr, cJSON *j_chr)
{
    cJSON *j_perms = cJSON_CreateArray();
    cJSON_AddItemToObject(j_chr, "perms", j_perms);
    if (chr->read != NULL || chr->static_value != NULL)
        cJSON_AddItemToArray(j_perms, cJSON_CreateString("pr"));
    if (chr->write != NULL)
        cJSON_AddItemToArray(j_perms, cJSON_CreateString("pw"));
    if (chr->can_notify)
        cJSON_AddItemToArray(j_perms, cJSON_CreateString("ev"));
}

void hk_accessories_serializer_chr(hk_chr_t *chr, cJSON *j_chrs)
{
    cJSON *j_chr = cJSON_CreateObject();
    cJSON_AddItemToArray(j_chrs, j_chr);

    char type[37] = {
        0,
    };
    sprintf(type, HAP_UUID, chr->type);
    cJSON_AddStringToObject(j_chr, "type", type);
    cJSON_AddNumberToObject(j_chr, "iid", chr->iid);

    hk_accessories_serializer_perms(chr, j_chr);
    hk_accessories_serializer_format(chr, j_chr);
    hk_accessories_serializer_value(chr, j_chr);
}

void hk_accessories_serializer_srv(hk_srv_t *srv, cJSON *j_srvs)
{
    cJSON *j_srv = cJSON_CreateObject();
    cJSON_AddItemToArray(j_srvs, j_srv);

    char type[37] = {
        0,
    };
    sprintf(type, HAP_UUID, srv->type); // todo: check if full type is needed
    cJSON_AddStringToObject(j_srv, "type", type);
    cJSON_AddNumberToObject(j_srv, "iid", srv->iid);
    cJSON_AddBoolToObject(j_srv, "primary", srv->primary);
    //cJSON_AddBoolToObject(j_srv, "hidden", srv->hidden);

    cJSON *j_chrs = cJSON_CreateArray();
    cJSON_AddItemToObject(j_srv, "chrs", j_chrs);

    hk_ll_foreach(srv->chrs, chr)
    {
        hk_accessories_serializer_chr(chr, j_chrs);
    }
}

void hk_accessories_serializer_accessory(hk_accessory_t *accessory, cJSON *j_accessories)
{
    cJSON *j_accessory = cJSON_CreateObject();
    cJSON_AddNumberToObject(j_accessory, "aid", accessory->aid);
    cJSON_AddItemToArray(j_accessories, j_accessory);

    cJSON *j_srvs = cJSON_CreateArray();
    cJSON_AddItemToObject(j_accessory, "srvs", j_srvs);

    hk_ll_foreach(accessory->srvs, srv)
    {
        hk_accessories_serializer_srv(srv, j_srvs);
    }
}

void hk_accessories_serializer_accessories(hk_mem *out)
{
    cJSON *j_root = cJSON_CreateObject();
    cJSON *j_accessories = cJSON_CreateArray();
    cJSON_AddItemToObject(j_root, "accessories", j_accessories);

    hk_accessory_t *accessories = hk_accessories_store_get_accessories();
    hk_ll_foreach(accessories, accessory)
    {
        hk_accessories_serializer_accessory(accessory, j_accessories);
    }

    char *serialized = cJSON_PrintUnformatted(j_root);
    hk_mem_append_string(out, (const char *)serialized);
    free(serialized);
    cJSON_Delete(j_root);
}