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
    service->chrs = NULL;

    hk_accessories->services = service;
}

void *hk_accessories_store_add_chr(hk_chr_types_t type, void *(*read)(), void (*write)(void *), bool can_notify)
{
    hk_chr_t *chr = hk_ll_new(hk_accessories->services->chrs);

    chr->type = type;
    chr->static_value = NULL;
    chr->read = read;
    chr->write = write;
    chr->can_notify = can_notify;

    hk_accessories->services->chrs = chr;

    return chr;
}

void hk_accessories_store_add_chr_static_read(hk_chr_types_t type, void *value)
{
    hk_chr_t *chr = hk_ll_new(hk_accessories->services->chrs);

    chr->type = type;
    chr->static_value = value;
    chr->read = NULL;
    chr->write = NULL;
    chr->can_notify = false;

    hk_accessories->services->chrs = chr;
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
                    if (service->chrs)
                    {
                        service->chrs = hk_ll_reverse(service->chrs);
                        hk_ll_foreach(service->chrs, chr)
                        {
                            chr->iid = ++iid;
                            chr->aid = accessory->aid;
                        }
                    }
                }
            }
        }
    }
}

hk_chr_t *hk_accessories_store_get_chr(size_t aid, size_t iid)
{
    if (hk_accessories)
    {
        hk_ll_foreach(hk_accessories, accessory)
        {
            if (aid == accessory->aid && accessory->services)
            {
                hk_ll_foreach(accessory->services, service)
                {
                    if (service->chrs)
                    {
                        hk_ll_foreach(service->chrs, chr)
                        {
                            if (iid == chr->iid)
                            {
                                return chr;
                            }
                        }
                    }
                }
            }
        }
    }

    return NULL;
}

hk_chr_t *hk_accessories_store_get_identify_chr()
{
    if (hk_accessories)
    {
        hk_ll_foreach(hk_accessories, accessory)
        {
            if (accessory->services)
            {
                hk_ll_foreach(accessory->services, service)
                {
                    if (service->chrs)
                    {
                        hk_ll_foreach(service->chrs, chr)
                        {
                            if (HK_CHR_IDENTIFY == chr->type)
                            {
                                return chr;
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