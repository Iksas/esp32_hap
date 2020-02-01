#include "hk_gatt_server.h"

// #include <host/ble_hs.h>
// #include <esp_nimble_hci.h>
// #include <nimble/ble.h>
// #include <nimble/nimble_port.h>
// #include <nimble/nimble_port_freertos.h>
// #include <host/ble_hs.h>
// #include <host/util/util.h>
#include "host/ble_uuid.h"
// #include "host/ble_att.h"
#include "host/ble_gatt.h"
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"
#include "../../include/homekit_characteristics.h"
#include "../../common/hk_accessories_store.h"
#include "hk_uuid_manager.h"
#include "hk_ble_types.h"

hk_ble_service_t *hk_services = NULL;
size_t hk_services_count = 0;

void *hk_gatt_read_accessory_information_service_signature()
{
    HK_LOGD("hk_gatt_read_accessory_information_service_signature");
    return 0;
}

void *hk_gatt_read_pair_setup()
{
    HK_LOGD("hk_gatt_read_pair_setup");
    return 0;
}

void hk_gatt_write_pair_setup(void *new_value)
{
    HK_LOGD("hk_gatt_write_pair_setup");
}

void *hk_gatt_read_pair_verify()
{
    HK_LOGD("hk_gatt_read_pair_verify");
    return 0;
}

void hk_gatt_write_pair_verify(void *new_value)
{
    HK_LOGD("hk_gatt_write_pair_verify");
}

void *hk_gatt_read_pairing_features()
{
    HK_LOGD("hk_gatt_read_pairing_features");
    return 0;
}

void *hk_gatt_read_pairing_pairings()
{
    HK_LOGD("hk_gatt_read_pairing_pairings");
    return 0;
}

void hk_gatt_write_pairing_pairings(void *new_value)
{
    HK_LOGD("hk_gatt_write_pairing_pairings");
}

void *hk_gatt_read_accessory_information()
{
    HK_LOGD("hk_gatt_read_accessory_information");
    return 0;
}

void hk_gatt_log_uuid(ble_uuid_t *u)
{
    char buffer[33];
    ble_uuid_to_str(u, buffer);
    HK_LOGD("UUID: %s", buffer);
}

static int hk_gatt_access_callback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    const ble_uuid128_t *uuid = BLE_UUID128(ctxt->chr->uuid);
    char buffer[33];
    ble_uuid_to_str(&uuid->u, buffer);
    int rc = 0;
    HK_LOGD("hk_gatt_access_callback %s", buffer);
    return rc;
}

void hk_gatt_characteristic_init(hk_ble_characteristic_t *characteristic, const ble_uuid_t *service_uuid, ble_uuid128_t *characteristic_uuid)
{
    characteristic->uuid = &characteristic_uuid->u;
    characteristic->access_cb = hk_gatt_access_callback;
    characteristic->flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ;

    size_t memory_size = 3 * sizeof(hk_ble_descriptor_t);
    characteristic->descriptors = (hk_ble_descriptor_t *)malloc(memory_size);
    memset((void *)characteristic->descriptors, 0, memory_size);

    characteristic->descriptors[0].uuid = &hk_uuid_manager_desciptor_instance_id.u;
    characteristic->descriptors[0].att_flags = BLE_ATT_F_READ;
    characteristic->descriptors[0].arg = (void *)service_uuid;
    characteristic->descriptors[0].access_cb = hk_gatt_access_callback;

    characteristic->descriptors[1].uuid = &hk_uuid_manager_descriptor_format.u;
    characteristic->descriptors[1].att_flags = BLE_ATT_F_READ;
    characteristic->descriptors[1].arg = (void *)service_uuid;
    characteristic->descriptors[1].access_cb = hk_gatt_access_callback;
}

void hk_gatt_init_characteristics(hk_service_t *service, hk_ble_service_t *ble_service)
{
    size_t characteristic_count = hk_ll_count(service->characteristics) + 2; // plus 2 because of service id and end marker
    size_t memory_size = characteristic_count * sizeof(hk_ble_characteristic_t);
    ble_service->characteristics = (hk_ble_characteristic_t *)malloc(memory_size);
    memset((void *)ble_service->characteristics, 0, memory_size);

    size_t characteristics_index = 0;
    // add service id characteristic
    hk_gatt_characteristic_init(
        (hk_ble_characteristic_t *)&ble_service->characteristics[characteristics_index],
        ble_service->uuid,
        (ble_uuid128_t *)&hk_uuid_manager_service_id);

    characteristics_index++;
    // add characteristics from accessory db
    hk_ll_foreach(service->characteristics, characteristic)
    {
        ble_uuid128_t *characteristic_uuid = hk_uuid_manager_get((uint8_t)characteristic->type);
        hk_gatt_characteristic_init(
            (hk_ble_characteristic_t *)&ble_service->characteristics[characteristics_index],
            ble_service->uuid,
            characteristic_uuid);
        characteristics_index++;
    };
}

int hk_gatt_srv_init()
{
    int rc = 0;

    HK_LOGD("Building configuration 1");
    ble_svc_gap_init();
    HK_LOGD("Building configuration2");
    ble_svc_gatt_init();

    hk_accessory_t *accessories = hk_accessories_store_get_accessories();
    if (hk_ll_count(accessories) != 1)
    {
        HK_LOGE("Using ble stack, only one accessory is allowed.");
        return -1;
    }

    hk_accessories_store_add_service(HK_SRV_HAP_PROTOCOL_INFORMATION, true, false);
    hk_accessories_store_add_characteristic(HK_CHR_VERSION, hk_gatt_read_accessory_information_service_signature, NULL, false);

    hk_accessories_store_add_service(HK_SRV_PARIRING, true, false);
    hk_accessories_store_add_characteristic(HK_CHR_PAIR_SETUP, hk_gatt_read_pair_setup, hk_gatt_write_pair_setup, false);
    hk_accessories_store_add_characteristic(HK_CHR_PAIR_VERIFY, hk_gatt_read_pair_verify, hk_gatt_write_pair_verify, false);
    hk_accessories_store_add_characteristic(HK_CHR_PAIRING_FEATURES, hk_gatt_read_pairing_features, NULL, false);
    hk_accessories_store_add_characteristic(HK_CHR_PAIRING_PAIRINGS, hk_gatt_read_pairing_pairings, hk_gatt_write_pairing_pairings, false);

    accessories = hk_accessories_store_get_accessories();
    hk_accessory_t accessory = accessories[0];
    size_t service_count = hk_ll_count(accessory.services) + 1; // plus 1 because of end marker
    size_t memory_size = service_count * sizeof(hk_ble_service_t);
    hk_services = malloc(memory_size);
    memset((void *)hk_services, 0, memory_size);

    HK_LOGD("Building configuration 3");
    size_t service_index = 0;
    hk_ll_foreach(accessory.services, service)
    {
        ble_uuid128_t *service_uuid = hk_uuid_manager_get((uint8_t)service->type);
        hk_services[service_index] = (hk_ble_service_t){
            .type = 1, //BLE_GATT_SVC_TYPE_PRIMARY
            .uuid = &service_uuid->u,
            //.characteristics = characteristics
        };

        hk_gatt_init_characteristics(service, &hk_services[service_index]);

        service_index++;
    };

    //    HK_LOGD("Descriptors4: %x", (uint)services[0].characteristics[0].descriptors[0].uuid);
    // hk_gatt_log_uuid(services[3].characteristics[0].uuid);
    // hk_gatt_log_uuid(services[2].uuid);
    // hk_gatt_log_uuid(services[1].uuid);
    // hk_gatt_log_uuid(services[0].uuid);
    //HK_LOGD("Data: %d", service_index);
    rc = ble_gatts_count_cfg(hk_services);
    if (rc != 0)
    {
        ESP_LOGE("GATT", "gatt_svr_init ble_gatts_count_cfg: %d", rc);
        return rc;
    }

    rc = ble_gatts_add_svcs(hk_services);
    if (rc != 0)
    {
        ESP_LOGE("GATT", "gatt_svr_init ble_gatts_add_svcs: %d", rc);
        return rc;
    }

    return rc;
}