#include "hk_nimble.h"

#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <esp_nimble_hci.h>
#include <host/ble_hs.h>
#include <host/util/util.h>

#include "../../utils/hk_logging.h"

#include "hk_gap.h"

void ble_store_config_init(void);

static void hk_nimble_on_reset(int reason)
{
    HK_LOGE("Resetting state; reason=%d\n", reason);
}

static void hk_nimble_on_sync(void)
{
    int res;

    res = ble_hs_util_ensure_addr(0);
    if(res != 0){
        HK_LOGE("Error");
    }

    /* Figure out address to use while advertising (no privacy for now) */
    uint8_t own_addr_type;
    res = ble_hs_id_infer_auto(0, &own_addr_type);
    if(res != 0){
        HK_LOGE("error determining address type; rc=%d", res);
    }

    /* Printing ADDR */
    uint8_t addr_val[6] = {0};
    res = ble_hs_id_copy_addr(own_addr_type, addr_val, NULL);
    if(res != 0){
        HK_LOGE("Error copying bluetooth address");
    }
    
    HK_LOGI("Got bluetooth address: %02x:%02x:%02x:%02x:%02x:%02x", addr_val[5], addr_val[4], addr_val[3], addr_val[2], addr_val[1], addr_val[0]);
    hk_gap_start_advertising(own_addr_type);
}

void hk_nimble_host_task(void *param)
{
    HK_LOGI("Nimble starting.");
    /* This function will return only when nimble_port_stop() is executed */
    nimble_port_run();
    HK_LOGI("Nimble stopping.");
    nimble_port_freertos_deinit();
}

void hk_nimble_init()
{
    HK_LOGD("Initializing nimble.");
    ESP_ERROR_CHECK(esp_nimble_hci_and_controller_init());
    nimble_port_init();

    /* Initialize the NimBLE host configuration. */
    ble_hs_cfg.reset_cb = hk_nimble_on_reset;
    ble_hs_cfg.sync_cb = hk_nimble_on_sync;
    ble_hs_cfg.store_status_cb = ble_store_util_status_rr;
    ble_hs_cfg.sm_io_cap = 3;
    ble_hs_cfg.sm_sc = 0;
}

void hk_nimble_start()
{
    HK_LOGD("Starting nimble.");
    /* XXX Need to have template for store */
    ble_store_config_init();

    nimble_port_freertos_init(hk_nimble_host_task);
}