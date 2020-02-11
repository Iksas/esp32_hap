#include "hk_gatt.h"

#include <host/ble_uuid.h>
#include <host/ble_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include <host/ble_hs.h>
//#include <esp_heap_caps.h> heap_caps_check_integrity_addr(0x3ffbd868, true);

#include "../../utils/hk_logging.h"
#include "../../utils/hk_tlv.h"
#include "../../common/hk_characteristics_properties.h"

#include "hk_uuid_manager.h"
#include "hk_formats_ble.h"

typedef struct ble_gatt_svc_def hk_ble_service_t;
typedef struct ble_gatt_chr_def hk_ble_characteristic_t;
typedef struct ble_gatt_dsc_def hk_ble_descriptor_t;

typedef struct
{
    int service_index;
    char service_id;
    int characteristic_index;
    uint16_t instance_id;
} hk_gatt_setup_info_t;

typedef struct
{
    char transaction_id;
    char last_opcode;
    const void* static_data;
    char service_index;
    char service_id;
    const ble_uuid128_t* service_uuid;
    char characteristic_index;
    hk_characteristic_types_t characteristic_type;
} hk_gatt_callback_info_t;

hk_gatt_setup_info_t *hk_gatt_setup_info;
hk_ble_service_t *hk_gatt_services = NULL;

static void hk_logu(const char* message, const ble_uuid128_t *uuid)
{
    char buffer[33];
    ble_uuid_to_str(&uuid->u, buffer);
    HK_LOGD("%s: %s", message, buffer);
}

static bool hk_gatt_cmp(const ble_uuid128_t *uuid1, const ble_uuid128_t *uuid2) {
    return ble_uuid_cmp(&uuid1->u, &uuid2->u) == 0;
}

static void hk_gatt_signature_request(const ble_uuid128_t *characteristic_uuid, hk_gatt_callback_info_t *callback_info, hk_mem* response)
{
    hk_tlv_t *tlv_data = NULL;
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_CHRARACTERISTIC_TYPE, (char*)characteristic_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_SERVICE_ID, callback_info->service_id);
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_SERVICE_TYPE, (char*)callback_info->service_uuid->value, 16);
    tlv_data = hk_tlv_add_uint16(tlv_data, HK_TLV_CHARACTERISTIC_PROPERTIES, 0x1);
    tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_USER_DESCRIPTION, "Version");
    tlv_data = hk_tlv_add_buffer(tlv_data, HK_TLV_PRESENTATION_FORMAT, hk_formats_ble_get(callback_info->characteristic_type), 7);
    // Characteristic Presentation Format: 0x19: string; 0x00: no exponent; 0x2700: unit = unitless; 0x01: Bluetooth SIG namespace; 0x0000: No description
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_VALID_RANGE, HK_CHR_VERSION);
    //tlv_data = hk_tlv_add_str(tlv_data, HK_TLV_STEP_VALUE, HK_CHR_VERSION);
    //size_t tlv_size = hk_tlv_get_size(tlv_data);

    hk_tlv_serialize(tlv_data, response);
}

static int hk_gatt_read_characteristic(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *characteristic_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_logu("Access to characteristic", characteristic_uuid);
    hk_gatt_callback_info_t* callback_info = (hk_gatt_callback_info_t*)arg;

    if(hk_gatt_cmp(characteristic_uuid, (ble_uuid128_t *)&hk_uuid_manager_service_id)){
        HK_LOGD("Returning instance id for service: %d", callback_info->service_id);
        uint16_t id = callback_info->service_id;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    } else {
        // todo: next 5 lines probably not needed
        // int buffer_len = OS_MBUF_PKTLEN(ctxt->om);
        // char buffer[buffer_len];
        // uint16_t out_len = 0;
        // rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
        // hk_log_print_bytewise("Request", buffer, out_len); 
        
        hk_mem *response = hk_mem_create();
        switch(callback_info->last_opcode){
            case 1:
                hk_gatt_signature_request(characteristic_uuid, callback_info, response);
                break;
            default:
                HK_LOGE("Unknown opcode.");
        }

        char out_buffer[5];
        out_buffer[0] = 0b00000010; // control field, always 1
        out_buffer[1] = callback_info->transaction_id;
        out_buffer[2] = 0; // status: zero for successful
        out_buffer[3] = response->size; // body len
        out_buffer[4] = 0; // body len
        
        hk_mem_prepend_buffer(response, out_buffer, 5);

        rc = os_mbuf_append(ctxt->om, response->ptr, response->size);

        hk_log_print_bytewise("Response", response->ptr, response->size); 
        //HK_LOGE("Ok> charid %d, b16 %d, b8 %d, lastop: %d, lastop: %d", character_id, body_len16, body_len8, last_opcode1, last_opcode2);
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_read_descriptor(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *characteristic_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_logu("Access to characteristic", characteristic_uuid);
    const ble_uuid128_t *descriptor_uuid = BLE_UUID128(ctxt->dsc->uuid);
    hk_logu("Access to descriptor", descriptor_uuid);
    hk_gatt_callback_info_t* callback_info = (hk_gatt_callback_info_t*)arg;

    if(hk_gatt_cmp(descriptor_uuid, (ble_uuid128_t *)&hk_uuid_manager_desciptor_instance_id)){
        HK_LOGD("Returning instance id for characteristic: %d", callback_info->characteristic_index);
        uint16_t id = callback_info->characteristic_index;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    } else {
        HK_LOGE("Operation not implemented.");
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_write_characteristic(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *characteristic_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_gatt_callback_info_t *callback_info = (hk_gatt_callback_info_t*)arg;
    hk_logu("Access to characteristic", characteristic_uuid);

    int buffer_len = OS_MBUF_PKTLEN(ctxt->om);
    char buffer[buffer_len];
    uint16_t out_len = 0;
    rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
    hk_log_print_bytewise("Request", buffer, out_len); 
    callback_info->last_opcode = buffer[1];
    callback_info->transaction_id = buffer[2];
    rc = rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    return rc;
}

static int hk_gatt_access_callback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR) {
        HK_LOGD("BLE_GATT_ACCESS_OP_READ_CHR");
        rc = hk_gatt_read_characteristic(ctxt, arg);
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC) {
        HK_LOGD("BLE_GATT_ACCESS_OP_READ_DSC");
        rc = hk_gatt_read_descriptor(ctxt, arg);
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        HK_LOGD("BLE_GATT_ACCESS_OP_WRITE_CHR");
        rc = hk_gatt_write_characteristic(ctxt, arg);
    } else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_DSC) {
        HK_LOGD("BLE_GATT_ACCESS_OP_WRITE_DSC");
        HK_LOGE("Operation not implemented.");
    } else{
        HK_LOGE("Operation not implemented.");
        return BLE_ATT_ERR_UNLIKELY;
    }

    return rc;
}

void *hk_gatt_alloc(void *ptr, size_t size)
{
    if (ptr == NULL)
    {
        ptr = malloc(size);
    }
    else
    {
        ptr = realloc(ptr, size);
    }

    return ptr;
}

hk_ble_service_t *hk_gatt_alloc_new_service()
{
    hk_gatt_setup_info->service_index++;
    hk_gatt_services = (hk_ble_service_t *)hk_gatt_alloc(
        hk_gatt_services,
        (hk_gatt_setup_info->service_index + 1) * sizeof(hk_ble_service_t));
    hk_ble_service_t *service = &hk_gatt_services[hk_gatt_setup_info->service_index];
    memset((void *)service, 0, sizeof(hk_ble_service_t));

    return service;
}

hk_ble_characteristic_t *hk_gatt_alloc_new_characteristic(hk_ble_service_t *current_service)
{
    hk_gatt_setup_info->characteristic_index++;
    current_service->characteristics = (hk_ble_characteristic_t *)hk_gatt_alloc(
        (void *)current_service->characteristics,
        (hk_gatt_setup_info->characteristic_index + 1) * sizeof(hk_ble_characteristic_t));
    hk_ble_characteristic_t *characteristic =
        (hk_ble_characteristic_t *)&current_service->characteristics[hk_gatt_setup_info->characteristic_index];

    memset(characteristic, 0, sizeof(hk_ble_characteristic_t));

    return characteristic;
}

void hk_gatt_characteristic_init(
    hk_ble_characteristic_t *characteristic,
    const ble_uuid_t *service_uuid,
    ble_uuid128_t *characteristic_uuid,
    hk_gatt_callback_info_t *callback_info)
{
    characteristic->uuid = &characteristic_uuid->u;
    characteristic->access_cb = hk_gatt_access_callback;
    characteristic->flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ;
    characteristic->arg = (void*)callback_info;

    size_t memory_size = 3 * sizeof(hk_ble_descriptor_t);
    characteristic->descriptors = (hk_ble_descriptor_t *)malloc(memory_size);
    memset((void *)characteristic->descriptors, 0, memory_size);

    characteristic->descriptors[0].uuid = &hk_uuid_manager_desciptor_instance_id.u;
    characteristic->descriptors[0].att_flags = BLE_ATT_F_READ;
    characteristic->descriptors[0].arg = (void *)callback_info;
    characteristic->descriptors[0].access_cb = hk_gatt_access_callback;

    characteristic->descriptors[1].uuid = &hk_uuid_manager_descriptor_format.u;
    characteristic->descriptors[1].att_flags = BLE_ATT_F_READ;
    characteristic->descriptors[1].arg = (void *)callback_info;
    characteristic->descriptors[1].access_cb = hk_gatt_access_callback;
}

void hk_gatt_init()
{
    HK_LOGD("Initializing GATT.");
    
    hk_gatt_setup_info = malloc(sizeof(hk_gatt_setup_info_t));
    hk_gatt_setup_info->service_index = -1;
    hk_gatt_setup_info->characteristic_index = -1;
    hk_gatt_setup_info->instance_id = 0;
}

void hk_gatt_add_service(hk_service_types_t service_type, bool primary, bool hidden)
{
    if (hk_gatt_setup_info->service_index >= 0)
    {
        // add end marker to characteristic array of last service
        hk_ble_service_t *last_service = &hk_gatt_services[hk_gatt_setup_info->service_index];
        hk_gatt_alloc_new_characteristic(last_service);
        hk_gatt_setup_info->characteristic_index = -1;
    }

    hk_ble_service_t *service = hk_gatt_alloc_new_service();

    ble_uuid128_t *service_uuid = hk_uuid_manager_get((uint8_t)service_type);
    service->type = 1;
    service->uuid = &service_uuid->u;

    // add service id characteristic
    hk_ble_characteristic_t *characteristic = hk_gatt_alloc_new_characteristic(service); // todo: combine in one function
    hk_gatt_callback_info_t *callback_info = (hk_gatt_callback_info_t*)malloc(sizeof(hk_gatt_callback_info_t));
    callback_info->static_data = NULL;
    callback_info->service_id 
        = hk_gatt_setup_info->service_id 
        = callback_info->characteristic_index
        = hk_gatt_setup_info->instance_id++;
    hk_gatt_characteristic_init(characteristic, service->uuid, (ble_uuid128_t *)&hk_uuid_manager_service_id, callback_info);
}

void *hk_gatt_add_characteristic(
    hk_characteristic_types_t characteristic_type,
    void *(*read)(),
    void (*write)(void *),
    bool can_notify)
{
    hk_ble_service_t *current_service = &hk_gatt_services[hk_gatt_setup_info->service_index];
    ble_uuid128_t *characteristic_uuid = hk_uuid_manager_get((uint8_t)characteristic_type);
    hk_ble_characteristic_t *characteristic = hk_gatt_alloc_new_characteristic(current_service);
    hk_gatt_callback_info_t *callback_info = (hk_gatt_callback_info_t*)malloc(sizeof(hk_gatt_callback_info_t));
    callback_info->static_data = NULL;
    callback_info->service_uuid = (ble_uuid128_t*)current_service->uuid;
    callback_info->service_index = hk_gatt_setup_info->service_index;
    callback_info->service_id = hk_gatt_setup_info->service_id ;
    callback_info->characteristic_index = hk_gatt_setup_info->instance_id++;
    callback_info->characteristic_type = characteristic_type;
    callback_info->transaction_id = -1;
    callback_info->last_opcode = -1;

    hk_gatt_characteristic_init(characteristic, current_service->uuid, characteristic_uuid, callback_info);
    return NULL;
}

void hk_gatt_add_characteristic_static_read(hk_characteristic_types_t characteristic_type, void *value)
{
    hk_ble_service_t *current_service = &hk_gatt_services[hk_gatt_setup_info->service_index];
    ble_uuid128_t *characteristic_uuid = hk_uuid_manager_get((uint8_t)characteristic_type);
    hk_ble_characteristic_t *characteristic = hk_gatt_alloc_new_characteristic(current_service);
    hk_gatt_callback_info_t *callback_info = (hk_gatt_callback_info_t*)malloc(sizeof(hk_gatt_callback_info_t));
    callback_info->static_data = value;
    callback_info->service_uuid = (ble_uuid128_t*)current_service->uuid;
    callback_info->service_index = hk_gatt_setup_info->service_index;
    callback_info->service_id = hk_gatt_setup_info->service_id ;
    callback_info->characteristic_index = hk_gatt_setup_info->instance_id++;
    callback_info->characteristic_type = characteristic_type;
    callback_info->transaction_id = -1;
    callback_info->last_opcode = -1;

    hk_gatt_characteristic_init(characteristic, current_service->uuid, characteristic_uuid, callback_info);
}

void hk_gatt_end_config()
{
    if (hk_gatt_setup_info->service_index >= 0)
    {
        // add end marker to characteristic array of last service
        hk_ble_service_t *last_service = &hk_gatt_services[hk_gatt_setup_info->service_index];
        hk_gatt_alloc_new_characteristic(last_service);
    }

    // add end marker to services array
    hk_gatt_alloc_new_service();

    free(hk_gatt_setup_info);
}

void hk_gatt_start()
{
    HK_LOGD("Starting GATT.");
    ble_svc_gatt_init();
    
    int rc = ble_gatts_count_cfg(hk_gatt_services);
    if (rc != 0) {
        HK_LOGE("gatt_svr_init ble_gatts_count_cfg: %d", rc);
        //return rc;
    }

    rc = ble_gatts_add_svcs(hk_gatt_services);
    if (rc != 0)
    {
        HK_LOGE("Error setting gatt config: %d", rc);
    }
}