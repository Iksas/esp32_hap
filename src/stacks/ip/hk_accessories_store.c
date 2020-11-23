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
    hk_accessories = hk_ll_init(hk_accessories);
    hk_accessories->srvs = NULL;
}

void hk_accessories_store_add_srv(hk_srv_types_t type, bool primary, bool hidden)
{
    hk_srv_t *srv = hk_ll_init(hk_accessories->srvs);

    srv->type = type;
    srv->primary = primary;
    srv->hidden = hidden;
    srv->chrs = NULL;

    hk_accessories->srvs = srv;
}

esp_err_t hk_accessories_store_add_chr(hk_chr_types_t type, esp_err_t (*read)(hk_mem* response), esp_err_t (*write)(hk_mem* request), bool can_notify, void **chr_ptr)
{
    hk_chr_t *chr = hk_ll_init(hk_accessories->srvs->chrs);

    chr->type = type;
    chr->static_value = NULL;
    chr->read = read;
    chr->write = write;
    chr->can_notify = can_notify;

    hk_accessories->srvs->chrs = chr;

    *chr_ptr = chr;
    return ESP_OK;
}

void hk_accessories_store_add_chr_static_read(hk_chr_types_t type, void *value)
{
    hk_chr_t *chr = hk_ll_init(hk_accessories->srvs->chrs);

    chr->type = type;
    chr->static_value = value;
    chr->read = NULL;
    chr->write = NULL;
    chr->can_notify = false;

    hk_accessories->srvs->chrs = chr;
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
            if (accessory->srvs)
            {
                accessory->srvs = hk_ll_reverse(accessory->srvs);
                hk_ll_foreach(accessory->srvs, srv)
                {
                    srv->iid = ++iid;
                    if (srv->chrs)
                    {
                        srv->chrs = hk_ll_reverse(srv->chrs);
                        hk_ll_foreach(srv->chrs, chr)
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
            if (aid == accessory->aid && accessory->srvs)
            {
                hk_ll_foreach(accessory->srvs, srv)
                {
                    if (srv->chrs)
                    {
                        hk_ll_foreach(srv->chrs, chr)
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
            if (accessory->srvs)
            {
                hk_ll_foreach(accessory->srvs, srv)
                {
                    if (srv->chrs)
                    {
                        hk_ll_foreach(srv->chrs, chr)
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
    HK_LOGW("Freeing accessories is not implemented.");
}