#include "hk_advertising.h"
#include "../../common/hk_accessories_store.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_ll.h"
#include "../../utils/hk_store.h"
#include "../../include/homekit_categories.h"

void hk_advertising_init(const char *name, hk_categories_t category, size_t config_version)
{
    hk_accessory_t *accessories = hk_accessories_store_get_accessories();
    hk_ll_foreach(accessories, accessory)
    {

    }
}

void hk_advertising_update_paired(bool initial)
{
   
}