#include "hk_accessories_serializer.h"

#include "../../include/homekit_services.h"
#include "../../include/homekit_characteristics.h"
#include "../../common/hk_accessories_store.h"
#include "../../common/hk_characteristics_properties.h"
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

void hk_accessories_serializer_value(hk_characteristic_t *characteristic, cJSON *j_characteristic)
{
    hk_format_t format = hk_characteristics_properties_get_type(characteristic->type);
    if (characteristic->read != NULL)
    {
        void *value = characteristic->read();
        cJSON_AddItemToObject(j_characteristic, "value", hk_accessories_serializer_format_value(format, value));
    }
    else if (characteristic->static_value != NULL)
    {
        cJSON_AddItemToObject(j_characteristic, "value", hk_accessories_serializer_format_value(format, characteristic->static_value));
    }
    else
    {
        if (characteristic->type != HK_CHR_IDENTIFY)
        {
            cJSON_AddNullToObject(j_characteristic, "value");
        }
    }
}

void hk_accessories_serializer_format(hk_characteristic_t *characteristic, cJSON *j_characteristic)
{
    hk_format_t format = hk_characteristics_properties_get_type(characteristic->type);
    switch (format)
    {
    case HK_FORMAT_BOOL:
        cJSON_AddStringToObject(j_characteristic, "format", "bool");
        break;
    case HK_FORMAT_UINT8:
        cJSON_AddStringToObject(j_characteristic, "format", "uint8");
        break;
    case HK_FORMAT_UINT32:
        cJSON_AddStringToObject(j_characteristic, "format", "uint32");
        break;
    case HK_FORMAT_UINT64:
        cJSON_AddStringToObject(j_characteristic, "format", "uint64");
        break;
    case HK_FORMAT_INT:
        cJSON_AddStringToObject(j_characteristic, "format", "int");
        break;
    case HK_FORMAT_FLOAT:
        cJSON_AddStringToObject(j_characteristic, "format", "float");
        break;
    case HK_FORMAT_STRING:
        cJSON_AddStringToObject(j_characteristic, "format", "string");
        break;
    case HK_FORMAT_TLV8:
        cJSON_AddStringToObject(j_characteristic, "format", "tlv8");
        break;
    case HK_FORMAT_DATA:
        cJSON_AddStringToObject(j_characteristic, "format", "data");
        break;
    default:
        HK_LOGE("Unknown format found: %d", format);
    }
}

void hk_accessories_serializer_perms(hk_characteristic_t *characteristic, cJSON *j_characteristic)
{
    cJSON *j_perms = cJSON_CreateArray();
    cJSON_AddItemToObject(j_characteristic, "perms", j_perms);
    if (characteristic->read != NULL || characteristic->static_value != NULL)
        cJSON_AddItemToArray(j_perms, cJSON_CreateString("pr"));
    if (characteristic->write != NULL)
        cJSON_AddItemToArray(j_perms, cJSON_CreateString("pw"));
    if (characteristic->can_notify)
        cJSON_AddItemToArray(j_perms, cJSON_CreateString("ev"));
}

void hk_accessories_serializer_characteristic(hk_characteristic_t *characteristic, cJSON *j_characteristics)
{
    cJSON *j_characteristic = cJSON_CreateObject();
    cJSON_AddItemToArray(j_characteristics, j_characteristic);

    char type[37] = {
        0,
    };
    sprintf(type, HAP_UUID, characteristic->type);
    cJSON_AddStringToObject(j_characteristic, "type", type);
    cJSON_AddNumberToObject(j_characteristic, "iid", characteristic->iid);

    hk_accessories_serializer_perms(characteristic, j_characteristic);
    hk_accessories_serializer_format(characteristic, j_characteristic);
    hk_accessories_serializer_value(characteristic, j_characteristic);
}

void hk_accessories_serializer_service(hk_service_t *service, cJSON *j_services)
{
    cJSON *j_service = cJSON_CreateObject();
    cJSON_AddItemToArray(j_services, j_service);

    char type[37] = {
        0,
    };
    sprintf(type, HAP_UUID, service->type); // todo: check if full type is needed
    cJSON_AddStringToObject(j_service, "type", type);
    cJSON_AddNumberToObject(j_service, "iid", service->iid);
    cJSON_AddBoolToObject(j_service, "primary", service->primary);
    //cJSON_AddBoolToObject(j_service, "hidden", service->hidden);

    cJSON *j_characteristics = cJSON_CreateArray();
    cJSON_AddItemToObject(j_service, "characteristics", j_characteristics);

    hk_ll_foreach(service->characteristics, characteristic)
    {
        hk_accessories_serializer_characteristic(characteristic, j_characteristics);
    }
}

void hk_accessories_serializer_accessory(hk_accessory_t *accessory, cJSON *j_accessories)
{
    cJSON *j_accessory = cJSON_CreateObject();
    cJSON_AddNumberToObject(j_accessory, "aid", accessory->aid);
    cJSON_AddItemToArray(j_accessories, j_accessory);

    cJSON *j_services = cJSON_CreateArray();
    cJSON_AddItemToObject(j_accessory, "services", j_services);

    hk_ll_foreach(accessory->services, service)
    {
        hk_accessories_serializer_service(service, j_services);
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