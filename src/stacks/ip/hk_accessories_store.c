#include "hk_accessories_store.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_ll.h"

hk_accessory_t *hk_accessories;

hk_accessory_t *hk_accessories_store_get_accessories()
{
    return hk_accessories;
}

void hk_accessories_store_add_accessory()
{
    hk_accessories = hk_ll_new(hk_accessories);
    hk_accessories->services = NULL;
}

void hk_accessories_store_add_service(hk_service_types_t type, bool primary, bool hidden)
{
    hk_service_t *service = hk_ll_new(hk_accessories->services);

    service->type = type;
    service->primary = primary;
    service->hidden = hidden;
    service->characteristics = NULL;

    hk_accessories->services = service;
}

void *hk_accessories_store_add_characteristic(hk_characteristic_types_t type, void *(*read)(), void (*write)(void *), bool can_notify)
{
    hk_characteristic_t *characteristic = hk_ll_new(hk_accessories->services->characteristics);

    characteristic->type = type;
    characteristic->static_value = NULL;
    characteristic->read = read;
    characteristic->write = write;
    characteristic->can_notify = can_notify;

    hk_accessories->services->characteristics = characteristic;

    return characteristic;
}

void hk_accessories_store_add_characteristic_static_read(hk_characteristic_types_t type, void *value)
{
    hk_characteristic_t *characteristic = hk_ll_new(hk_accessories->services->characteristics);

    characteristic->type = type;
    characteristic->static_value = value;
    characteristic->read = NULL;
    characteristic->write = NULL;
    characteristic->can_notify = false;

    hk_accessories->services->characteristics = characteristic;
}

void hk_accessories_store_end_config()
{
    if (hk_accessories)
    {
        size_t aid = 0;
        size_t iid = 0;
        hk_accessories = hk_ll_reverse(hk_accessories);
        hk_ll_foreach(hk_accessories, accessory)
        {
            iid = 0;
            accessory->aid = ++aid;
            if (accessory->services)
            {
                accessory->services = hk_ll_reverse(accessory->services);
                hk_ll_foreach(accessory->services, service)
                {
                    service->iid = ++iid;
                    if (service->characteristics)
                    {
                        service->characteristics = hk_ll_reverse(service->characteristics);
                        hk_ll_foreach(service->characteristics, characteristic)
                        {
                            characteristic->iid = ++iid;
                            characteristic->aid = accessory->aid;
                        }
                    }
                }
            }
        }
    }
}

hk_characteristic_t *hk_accessories_store_get_characteristic(size_t aid, size_t iid)
{
    if (hk_accessories)
    {
        hk_ll_foreach(hk_accessories, accessory)
        {
            if (aid == accessory->aid && accessory->services)
            {
                hk_ll_foreach(accessory->services, service)
                {
                    if (service->characteristics)
                    {
                        hk_ll_foreach(service->characteristics, characteristic)
                        {
                            if (iid == characteristic->iid)
                            {
                                return characteristic;
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

hk_characteristic_t *hk_accessories_store_get_identify_characteristic()
{
    if (hk_accessories)
    {
        hk_ll_foreach(hk_accessories, accessory)
        {
            if (accessory->services)
            {
                hk_ll_foreach(accessory->services, service)
                {
                    if (service->characteristics)
                    {
                        hk_ll_foreach(service->characteristics, characteristic)
                        {
                            if (HK_CHR_IDENTIFY == characteristic->type)
                            {
                                return characteristic;
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

void hk_accessories_free()
{
    // we do not free ressources, because this method is used in unit testing only
    hk_accessories = NULL;
}