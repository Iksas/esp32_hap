#include "hk_gap.h"

#include <services/gap/ble_svc_gap.h>

#include "../../utils/hk_logging.h"
#include "../../utils/hk_store.h"
#include "../../utils/hk_util.h"
#include "../../common/hk_accessory_id.h"
#include "../../common/hk_pairings_store.h"
#include "../../common/hk_global_state.h"
#include "../../crypto/hk_chacha20poly1305.h"

#include "hk_connection_security.h"
#include "hk_connection.h"

static uint8_t hk_gap_own_addr_type;
const char *hk_gap_name; // todo: move to config
size_t hk_gap_category;  // todo: move to config, type to uint16_t
hk_mem *hk_gap_broadcast_key = NULL;

static void hk_gap_disconnect(uint16_t handle)
{
    hk_connection_free(handle);
}

static esp_err_t hk_gap_get_peer_address(int handle, hk_mem *address)
{
    struct ble_gap_conn_desc conn_desc;
    esp_err_t ret = ble_gap_conn_find(handle, &conn_desc);
    if (!ret)
    {
        hk_mem_set(address, 18);
        sprintf(address->ptr, "%x.%x.%x.%x.%x.%x",
                conn_desc.peer_id_addr.val[0], conn_desc.peer_id_addr.val[1],
                conn_desc.peer_id_addr.val[2], conn_desc.peer_id_addr.val[3],
                conn_desc.peer_id_addr.val[4], conn_desc.peer_id_addr.val[5]);
    }

    return ret;
}

/**
 * The nimble host executes this callback when a GAP event occurs.  The
 * application associates a GAP event callback with each connection that forms.
 * bleprph uses the same callback for all connections.
 *
 * @param event                 The type of event being signalled.
 * @param ctxt                  Various information pertaining to the event.
 * @param arg                   Application-specified argument; unused by
 *                                  bleprph.
 *
 * @return                      0 if the application successfully handled the
 *                                  event; nonzero on failure.  The semantics
 *                                  of the return code is specific to the
 *                                  particular GAP event being signalled.
 */
static int hk_gap_gap_event(struct ble_gap_event *event, void *arg)
{
    int rc = 0;

    switch (event->type)
    {
    case BLE_GAP_EVENT_CONNECT:
        HK_LOGV("Connect event; status=%d ", event->connect.status);
        if (event->connect.status == 0)
        {
            hk_mem *address = hk_mem_init();
            hk_gap_get_peer_address(event->connect.conn_handle, address);
            hk_connection_init(event->connect.conn_handle, address);
            hk_mem_free(address);
        }
        else
        {
            //Connection failed; resume advertising.
            hk_gap_start_advertising(hk_gap_own_addr_type);
        }
        break;
    case BLE_GAP_EVENT_DISCONNECT:
        HK_LOGD("Disconnect event; reason=%d ", event->disconnect.reason);
        hk_gap_disconnect(event->disconnect.conn.conn_handle);
        hk_gap_start_advertising(hk_gap_own_addr_type);
        break;
    case BLE_GAP_EVENT_CONN_UPDATE:
        HK_LOGV("connection updated; status=%d ", event->conn_update.status);
        rc = 0;
        break;
    case BLE_GAP_EVENT_ADV_COMPLETE:
        HK_LOGD("advertise complete; reason=%d", event->adv_complete.reason);
        hk_gap_start_advertising(hk_gap_own_addr_type);
        rc = 0;
        break;
    case BLE_GAP_EVENT_ENC_CHANGE:
        HK_LOGD("encryption change event; status=%d ", event->enc_change.status);
        rc = 0;
        break;
    case BLE_GAP_EVENT_SUBSCRIBE:
        HK_LOGV("subscribe event; conn_handle=%d attr_handle=%d "
                "reason=%d prevn=%d curn=%d previ=%d curi=%d\n",
                event->subscribe.conn_handle,
                event->subscribe.attr_handle,
                event->subscribe.reason,
                event->subscribe.prev_notify,
                event->subscribe.cur_notify,
                event->subscribe.prev_indicate,
                event->subscribe.cur_indicate);
        rc = 0;
        break;
    case BLE_GAP_EVENT_MTU:
        // HK_LOGD("mtu update event; conn_handle=%d cid=%d mtu=%d",
        //         event->mtu.conn_handle,
        //         event->mtu.channel_id,
        //         event->mtu.value);
        hk_connection_mtu_set(event->mtu.conn_handle, event->mtu.value);
        rc = 0;
        break;
    case BLE_GAP_EVENT_REPEAT_PAIRING:
        HK_LOGD("Repeat pairing");
        rc = 0;
        break;
    case BLE_GAP_EVENT_PASSKEY_ACTION:
        HK_LOGD("PASSKEY_ACTION_EVENT started");
        rc = 0;
        break;
    }

    return rc;
}

static esp_err_t hk_gap_start_advertising_internal(hk_mem *manufacturer_data, bool send_name)
{
    int err;

    if (ble_gap_adv_active())
    {
        HK_LOGD("Stopping advertising.");
        err = ble_gap_adv_stop();
        if (err)
        {
            HK_LOGE("Could not stop advertising. Errorcode: %d", err);
            return ESP_FAIL;
        }
    }

    struct ble_hs_adv_fields fields;
    memset(&fields, 0, sizeof fields);
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP; // Discoverability in forthcoming advertisement (general) // BLE-only (BR/EDR unsupported)
    if (send_name)
    {
        fields.name = (uint8_t *)hk_gap_name;
        fields.name_len = strlen(hk_gap_name);
        fields.name_is_complete = 1;
    }
    fields.mfg_data = (uint8_t *)manufacturer_data->ptr;
    fields.mfg_data_len = manufacturer_data->size;

    err = ble_gap_adv_set_fields(&fields);
    if (err)
    {
        HK_LOGE("Could not start advertising, because fields could not be set. Errorcode: %d", err);
        return ESP_FAIL;
    }

    /* Begin advertising. */
    struct ble_gap_adv_params adv_params;
    memset(&adv_params, 0, sizeof adv_params);
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
    //adv_params.itvl_min = 20;
    err = ble_gap_adv_start(hk_gap_own_addr_type, NULL, BLE_HS_FOREVER, &adv_params, hk_gap_gap_event, NULL);
    if (err)
    {
        HK_LOGE("Could not start advertising. Errorcode: %d", err);
        return ESP_FAIL;
    }

    HK_LOGD("Advertising started.");
    return ESP_OK;
}

esp_err_t hk_gap_start_advertising()
{
    HK_LOGD("Starting advertising.");
    esp_err_t ret = ESP_OK;

    hk_mem *accessory_id = hk_mem_init();
    hk_mem *manufacturer_data = hk_mem_init();

    bool has_pairing = false;
    hk_pairings_store_has_pairing(&has_pairing);

    uint16_t company_id = 0x4c;
    uint16_t global_state = hk_global_state_get();
    uint8_t type = 0x06;
    uint8_t stl = 0x2d;
    uint8_t sf = has_pairing ? 0x00 : 0x01;
    uint8_t configuration = hk_store_configuration_get();
    uint8_t ble = 0x02;
    hk_accessory_id_get(accessory_id);

    hk_mem_append_buffer(manufacturer_data, &company_id, 2);
    hk_mem_append_buffer(manufacturer_data, &type, 1);
    hk_mem_append_buffer(manufacturer_data, &stl, 1);
    hk_mem_append_buffer(manufacturer_data, &sf, 1);
    hk_mem_append(manufacturer_data, accessory_id);
    hk_mem_append_buffer(manufacturer_data, &hk_gap_category, 2);
    hk_mem_append_buffer(manufacturer_data, &global_state, 2);
    hk_mem_append_buffer(manufacturer_data, &configuration, 1);
    hk_mem_append_buffer(manufacturer_data, &ble, 1);

    //HK_LOGD("manu: %s", hk_mem_to_debug_string(manufacturer_data));
    RUN_AND_CHECK(ret, hk_gap_start_advertising_internal, manufacturer_data, true);

    hk_mem_free(accessory_id);
    hk_mem_free(manufacturer_data);

    return ret;
}

void hk_gap_broadcast_key_set(hk_mem *broadcast_key)
{
    HK_LOGD("Broadcast key length: %d", broadcast_key->size);
    hk_mem_set_mem(hk_gap_broadcast_key, broadcast_key);
}

esp_err_t hk_gap_start_advertising_change(uint16_t chr_index, hk_mem *value)
{
    if (hk_gap_broadcast_key->size < 1)
    {
        HK_LOGW("No broadcast key until now. Canceling advertising for change. Probably Protocol configuration was not called by controller.");
        return ESP_OK;
    }

    HK_LOGD("Starting advertising change for chr: %d", chr_index);
    esp_err_t ret = ESP_OK;
    hk_mem *accessory_id = hk_mem_init();
    hk_mem *manufacturer_data = hk_mem_init();
    hk_mem *data_to_encrypt = hk_mem_init();
    hk_mem *encrypted = hk_mem_init();

    uint8_t empty = 0;
    uint16_t company_id = 0x4c;
    uint16_t global_state = hk_global_state_get();
    uint8_t type = 0x11;
    uint8_t stl = 0x36; // subtype and length (001 for subtype plus 22 as length => 0011 0110 => 36)

    hk_accessory_id_get(accessory_id);

    while (value->size < 8)
    {
        hk_mem_append_buffer(value, &empty, 1);
    }

    hk_mem_append_buffer(data_to_encrypt, &global_state, 2);
    hk_mem_append_buffer(data_to_encrypt, &chr_index, 2);
    hk_mem_append(data_to_encrypt, value);

    // HK_LOGD("encrypting: %s %s %s",
    //         hk_mem_to_debug_string(hk_gap_broadcast_key),
    //         hk_mem_to_debug_string(data_to_encrypt),
    //         hk_mem_to_debug_string(encrypted));
    RUN_AND_CHECK(ret, hk_chacha20poly1305_encrypt, hk_gap_broadcast_key, "GS", data_to_encrypt, encrypted);

    //HK_LOGD("encrypting done: %s", hk_mem_to_debug_string(encrypted));
    if (!ret)
    {
        HK_LOGD("Current message length: %d", encrypted->size);
        hk_mem_set(encrypted, 16);

        hk_mem_append_buffer(manufacturer_data, &company_id, 2);
        hk_mem_append_buffer(manufacturer_data, &type, 1);
        hk_mem_append_buffer(manufacturer_data, &stl, 1);
        hk_mem_append(manufacturer_data, accessory_id);
        hk_mem_append(manufacturer_data, encrypted);

        HK_LOGD("Starting internal");
        hk_log_print_bytewise("manu", manufacturer_data->ptr, manufacturer_data->size, false);
        RUN_AND_CHECK(ret, hk_gap_start_advertising_internal, manufacturer_data, false);
    }

    hk_mem_free(accessory_id);
    hk_mem_free(manufacturer_data);
    hk_mem_free(data_to_encrypt);
    hk_mem_free(encrypted);

    HK_LOGD("Done starting advertising for change.");
    return ret;
}

void hk_gap_address_set(uint8_t own_addr_type)
{
    hk_gap_own_addr_type = own_addr_type;
}

void hk_gap_init(const char *name, size_t category, size_t config_version)
{
    HK_LOGD("Initializing GAP.");

    hk_gap_broadcast_key = hk_mem_init();

    ble_svc_gap_init();
    hk_gap_name = name;
    hk_gap_category = category;
    int ret = ble_svc_gap_device_name_set(name);
    if (ret != ESP_OK)
    {
        HK_LOGE("Error setting name for advertising.");
        return;
    }
}

void hk_gap_terminate_connection(uint16_t handle)
{
    ble_gap_terminate(handle, BLE_ERR_REM_USER_CONN_TERM);
}