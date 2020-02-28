#include "hk_gatt.h"

#include <host/ble_uuid.h>
#include <host/ble_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include <host/ble_hs.h>
//#include <esp_heap_caps.h> heap_caps_check_integrity_addr(0x3ffbd868, true);

#include "../../utils/hk_logging.h"

#include "hk_session.h"
#include "hk_uuid_manager.h"
#include "operations/hk_characteristic_signature_read.h"
#include "operations/hk_characteristic_write.h"
#include "operations/hk_characteristic_read.h"
#include "operations/hk_characteristic_timed_write.h"
#include "operations/hk_characteristic_execute_write.h"
#include "operations/hk_service_signature_read.h"
#include "operations/hk_characteristic_configuration.h"
#include "operations/hk_protocol_configuration.h"

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

hk_gatt_setup_info_t *hk_gatt_setup_info;
hk_ble_service_t *hk_gatt_services = NULL;

static void hk_logu(const char *message, const ble_uuid128_t *uuid)
{
    char buffer[33];
    ble_uuid_to_str(&uuid->u, buffer);
    HK_LOGD("%s: %s", message, buffer);
}

static bool hk_gatt_cmp(const ble_uuid128_t *uuid1, const ble_uuid128_t *uuid2)
{
    return ble_uuid_cmp(&uuid1->u, &uuid2->u) == 0;
}

static int hk_gatt_read_characteristic(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *characteristic_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_logu("Request to read characteristic", characteristic_uuid);
    hk_session_t *session = (hk_session_t *)arg;

    if (hk_gatt_cmp(characteristic_uuid, (ble_uuid128_t *)&hk_uuid_manager_service_id))
    {
        HK_LOGD("Returning instance id for service: %d", session->service_id);
        uint16_t id = session->service_id;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    }
    else
    {
        // todo: next 5 lines probably not needed
        // int buffer_len = OS_MBUF_PKTLEN(ctxt->om);
        // char buffer[buffer_len];
        // uint16_t out_len = 0;
        // rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
        // hk_log_print_bytewise("Request", buffer, out_len, false);

        hk_mem *response = hk_mem_create();
        switch (session->last_opcode)
        {
        case 1:
            hk_characteristic_signature_read_response(characteristic_uuid, session, response);
            break;
        case 2:
            hk_characteristic_write_response(characteristic_uuid, session, response);
            break;
        case 3:
            hk_characteristic_read_response(characteristic_uuid, session, response);
            break;
        case 4:
            hk_characteristic_timed_write_response(characteristic_uuid, session, response);
            break;
        case 5:
            hk_characteristic_execute_write_response(characteristic_uuid, session, response);
            break;
        case 6:
            hk_service_signature_read_response(characteristic_uuid, session, response);
            break;
        case 7:
            hk_characteristic_configuration_response(characteristic_uuid, session, response);
            break;
        case 8:
            hk_protocol_configuration_response(characteristic_uuid, session, response);
            break;
        default:
            HK_LOGE("Unknown opcode.");
        }

        uint8_t out_buffer[3];
        out_buffer[0] = 0b00000010; // control field, always 1
        out_buffer[1] = session->transaction_id;
        out_buffer[2] = 0; // status: zero for successful
        if (response->size > 0)
        {
            //write body length
            uint16_t body_length = response->size;
            hk_mem_prepend_buffer(response, (char*)&body_length, sizeof(uint16_t));
            hk_mem_prepend_buffer(response, (char*)out_buffer, 3);

            //write body
            rc = os_mbuf_append(ctxt->om, response->ptr, response->size);
            hk_log_print_bytewise("Response with body", response->ptr, response->size, false);

        } else {
            rc = os_mbuf_append(ctxt->om, out_buffer, 3);
            hk_log_print_bytewise("Response without body", (char*)out_buffer, 3, false);
        }
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_read_descriptor(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *characteristic_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_logu("Read descriptor of  characteristic", characteristic_uuid);
    const ble_uuid128_t *descriptor_uuid = BLE_UUID128(ctxt->dsc->uuid);
    hk_logu("with id", descriptor_uuid);
    hk_session_t *session = (hk_session_t *)arg;

    if (hk_gatt_cmp(descriptor_uuid, (ble_uuid128_t *)&hk_uuid_manager_desciptor_instance_id))
    {
        HK_LOGD("Returning instance id for characteristic: %d", session->characteristic_index);
        uint16_t id = session->characteristic_index;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    }
    else
    {
        HK_LOGE("Operation not implemented.");
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_write_characteristic(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *characteristic_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_session_t *session = (hk_session_t *)arg;
    hk_logu("Request to write characteristic", characteristic_uuid);

    uint8_t buffer_len = OS_MBUF_PKTLEN(ctxt->om);
    char buffer[buffer_len];
    uint16_t out_len = 0;
    rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
    hk_log_print_bytewise("Request", buffer, out_len, false);
    session->last_opcode = buffer[1];
    session->transaction_id = buffer[2];
    uint16_t body_length = buffer[5];
    
    hk_mem_set(session->request, 0);
    hk_mem_append_buffer(session->request, buffer + 7, body_length);

    rc = rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    return rc;
}

static int hk_gatt_access_callback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_READ_CHR");
        rc = hk_gatt_read_characteristic(ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_READ_DSC");
        rc = hk_gatt_read_descriptor(ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_WRITE_CHR");
        rc = hk_gatt_write_characteristic(ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_DSC)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_WRITE_DSC");
        HK_LOGE("Operation not implemented.");
    }
    else
    {
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
    ble_gatt_chr_flags flags,
    hk_session_t *session,
    bool add_descriptors)
{
    characteristic->uuid = &characteristic_uuid->u;
    characteristic->access_cb = hk_gatt_access_callback;
    characteristic->flags = flags;
    characteristic->arg = (void *)session;

    size_t memory_size = 3 * sizeof(hk_ble_descriptor_t);
    characteristic->descriptors = (hk_ble_descriptor_t *)malloc(memory_size);
    memset((void *)characteristic->descriptors, 0, memory_size);

    characteristic->descriptors[0].uuid = &hk_uuid_manager_desciptor_instance_id.u;
    characteristic->descriptors[0].att_flags = BLE_ATT_F_READ;
    characteristic->descriptors[0].arg = (void *)session;
    characteristic->descriptors[0].access_cb = hk_gatt_access_callback;

    // characteristic->descriptors[1].uuid = &hk_uuid_manager_descriptor_format.u;
    // characteristic->descriptors[1].att_flags = BLE_ATT_F_READ;
    // characteristic->descriptors[1].arg = (void *)session;
    // characteristic->descriptors[1].access_cb = hk_gatt_access_callback;
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
    hk_session_t *session = (hk_session_t *)malloc(sizeof(hk_session_t));
    session->static_data = NULL;
    session->service_id = hk_gatt_setup_info->service_id = session->characteristic_index = hk_gatt_setup_info->instance_id++;
    hk_gatt_characteristic_init(characteristic, service->uuid, (ble_uuid128_t *)&hk_uuid_manager_service_id, BLE_GATT_CHR_F_READ, session, false);
}

void *hk_gatt_add_characteristic(
    hk_characteristic_types_t characteristic_type,
    void *(*read)(size_t*),
    void *(*write)(void *, size_t, size_t*),
    bool can_notify,
    int16_t min_length,
    int16_t max_length)
{
    hk_ble_service_t *current_service = &hk_gatt_services[hk_gatt_setup_info->service_index];
    ble_uuid128_t *characteristic_uuid = hk_uuid_manager_get((uint8_t)characteristic_type);
    hk_ble_characteristic_t *characteristic = hk_gatt_alloc_new_characteristic(current_service);
    hk_session_t *session = (hk_session_t *)malloc(sizeof(hk_session_t));
    session->static_data = NULL;
    session->service_uuid = (ble_uuid128_t *)current_service->uuid;
    session->service_index = hk_gatt_setup_info->service_index;
    session->service_id = hk_gatt_setup_info->service_id;
    session->characteristic_index = hk_gatt_setup_info->instance_id++;
    session->characteristic_type = characteristic_type;
    session->transaction_id = -1;
    session->last_opcode = -1;
    session->read_callback = read;
    session->write_callback = write;
    session->max_length = max_length;
    session->min_length = min_length;
    session->request = hk_mem_create();

    ble_gatt_chr_flags flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ;

    if (can_notify)
    {
        flags |= BLE_GATT_CHR_F_INDICATE;
    }

    hk_gatt_characteristic_init(
        characteristic,
        current_service->uuid,
        characteristic_uuid,
        flags,
        session, true);
    return NULL;
}

void hk_gatt_add_characteristic_static_read(hk_characteristic_types_t characteristic_type, const char *value)
{
    hk_ble_service_t *current_service = &hk_gatt_services[hk_gatt_setup_info->service_index];
    ble_uuid128_t *characteristic_uuid = hk_uuid_manager_get((uint8_t)characteristic_type);
    hk_ble_characteristic_t *characteristic = hk_gatt_alloc_new_characteristic(current_service);
    hk_session_t *session = (hk_session_t *)malloc(sizeof(hk_session_t));
    session->static_data = value;
    session->service_uuid = (ble_uuid128_t *)current_service->uuid;
    session->service_index = hk_gatt_setup_info->service_index;
    session->service_id = hk_gatt_setup_info->service_id;
    session->characteristic_index = hk_gatt_setup_info->instance_id++;
    session->characteristic_type = characteristic_type;
    session->transaction_id = -1;
    session->last_opcode = -1;
    session->max_length = -1;
    session->min_length = -1;
    session->request = hk_mem_create();

    hk_gatt_characteristic_init(
        characteristic,
        current_service->uuid,
        characteristic_uuid,
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ,
        session, true);
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
    if (rc != 0)
    {
        HK_LOGE("gatt_svr_init ble_gatts_count_cfg: %d", rc);
        //return rc;
    }

    rc = ble_gatts_add_svcs(hk_gatt_services);
    if (rc != 0)
    {
        HK_LOGE("Error setting gatt config: %d", rc);
    }
}