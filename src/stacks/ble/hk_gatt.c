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
#include "operations/hk_chr_signature_read.h"
#include "operations/hk_chr_write.h"
#include "operations/hk_chr_read.h"
#include "operations/hk_chr_timed_write.h"
#include "operations/hk_chr_execute_write.h"
#include "operations/hk_srv_signature_read.h"
#include "operations/hk_chr_configuration.h"
#include "operations/hk_protocol_configuration.h"

typedef struct ble_gatt_svc_def hk_ble_srv_t;
typedef struct ble_gatt_chr_def hk_ble_chr_t;
typedef struct ble_gatt_dsc_def hk_ble_descriptor_t;

hk_session_setup_info_t *hk_gatt_setup_info;
hk_ble_srv_t *hk_gatt_srvs = NULL;

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

static int hk_gatt_read_ble_descriptor(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_logu("Read descriptor of  chr", chr_uuid);
    const ble_uuid128_t *descriptor_uuid = BLE_UUID128(ctxt->dsc->uuid);
    hk_logu("with id", descriptor_uuid);
    hk_session_t *session = (hk_session_t *)arg;

    if (hk_gatt_cmp(descriptor_uuid, (ble_uuid128_t *)&hk_uuid_manager_desciptor_instance_id))
    {
        HK_LOGD("Returning instance id for chr: %d", session->chr_index);
        uint16_t id = session->chr_index;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    }
    else
    {
        HK_LOGE("Operation not implemented.");
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_write_ble_chr(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_session_t *session = (hk_session_t *)arg;
    hk_logu("Request to write ble chr", chr_uuid);

    uint8_t buffer_len = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t buffer[buffer_len];
    uint16_t out_len = 0;
    rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
    uint8_t control_field = buffer[0];
    if (control_field && 0b01000000) // according specification bit 7 (zero based) should be set to 1. But it isnt????
    {
        // continuation
        hk_mem_append_buffer(session->request, buffer + 2, out_len - 2); //continuation is preceeded by controlfield and transaction id
        HK_LOGD("Received continuation %d of %d.", session->request->size, session->request_length);
    }
    else
    {
        hk_log_print_bytewise("Received start", (char *)buffer, out_len > 5 ? 7 : 5, false);
        session->last_opcode = buffer[1];
        session->transaction_id = buffer[2];

        hk_mem_set(session->request, 0);
        if (out_len > 5)
        {
            memcpy(&session->request_length, buffer + 5, 2);
            hk_mem_append_buffer(session->request, buffer + 7, out_len - 7); // -7 because of PDU start
        }
        else
        {
            session->request_length = 0;
        }
    }

    if (session->request_length == session->request->size)
    {
        hk_log_print_bytewise("Executing", session->request->ptr, session->request->size, false);
        hk_mem_set(session->response, 0);
        session->response_sent = 0;
        // todo, execute functionshk_mem *response = hk_mem_create();
        switch (session->last_opcode)
        {
        case 1:
            hk_chr_signature_read_response(chr_uuid, session);
            break;
        case 2:
            hk_chr_write_response(chr_uuid, session);
            break;
        case 3:
            hk_chr_read_response(chr_uuid, session);
            break;
        case 4:
            hk_chr_timed_write_response(chr_uuid, session);
            break;
        case 5:
            hk_chr_execute_write_response(chr_uuid, session);
            break;
        case 6:
            hk_srv_signature_read_response(chr_uuid, session);
            break;
        case 7:
            hk_chr_configuration_response(chr_uuid, session);
            break;
        case 8:
            hk_protocol_configuration_response(chr_uuid, session);
            break;
        default:
            HK_LOGE("Unknown opcode.");
        }
    }
    else
    {
        HK_LOGD("Received %d of %d. Waiting for continuation of fragmentation.", session->request->size, session->request_length);
    }

    rc = rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;

    return rc;
}

static int hk_gatt_read_ble_chr(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_logu("Request to read ble characteristic", chr_uuid);
    hk_session_t *session = (hk_session_t *)arg;

    if (hk_gatt_cmp(chr_uuid, (ble_uuid128_t *)&hk_uuid_manager_srv_id))
    {
        HK_LOGD("Returning instance id for service: %d", session->srv_id);
        uint16_t id = session->srv_id;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    }
    else
    {
        hk_mem *response = hk_mem_create();
        uint8_t control_field = session->response_sent < 1 ? 0b00000010 : 0b10000010;
        hk_mem_append_buffer(response, (char *)&control_field, 1);

        hk_mem_append_buffer(response, (char *)&session->transaction_id, 1);

        if (session->response_sent < 1) // no continuation
        {
            uint8_t status = 0; // status is zero, success
            hk_mem_append_buffer(response, (char *)&status, 1);

            if (session->response->size > 0) // has body
            {
                uint16_t size = session->response->size;
                hk_mem_insert_buffer(response, (char *)&size, 2, 3);
            }
        }

        if (session->response->size > 0)
        {
            size_t response_size = session->response->size - session->response_sent;
            if (response_size > 246)
            {
                response_size = 246;
            }

            hk_mem_append_buffer(response, session->response->ptr + session->response_sent, response_size);
            session->response_sent += response_size;
        }

        hk_log_print_bytewise("Response", response->ptr, response->size, false);
        rc = os_mbuf_append(ctxt->om, response->ptr, response->size);
        HK_LOGD("Sent %d of %d", session->response_sent, session->response->size);

        if(session->response_sent == session->response->size){ //todo: this should not be needed, but homkit controller keeps asking even if everything was sent. Mybe a problem with fragmentation?
            session->response_sent = 0;
        }

        hk_mem_free(response);
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_access_callback(uint16_t conn_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_READ_CHR");
        rc = hk_gatt_read_ble_chr(ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_READ_DSC");
        rc = hk_gatt_read_ble_descriptor(ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        HK_LOGD("BLE_GATT_ACCESS_OP_WRITE_CHR");
        rc = hk_gatt_write_ble_chr(ctxt, arg);
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

hk_ble_srv_t *hk_gatt_alloc_new_srv()
{
    hk_gatt_setup_info->srv_index++;
    hk_gatt_srvs = (hk_ble_srv_t *)hk_gatt_alloc(
        hk_gatt_srvs,
        (hk_gatt_setup_info->srv_index + 1) * sizeof(hk_ble_srv_t));
    hk_ble_srv_t *srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    memset((void *)srv, 0, sizeof(hk_ble_srv_t));

    return srv;
}

hk_ble_chr_t *hk_gatt_alloc_new_chr(hk_ble_srv_t *current_srv)
{
    hk_gatt_setup_info->chr_index++;
    current_srv->characteristics = (hk_ble_chr_t *)hk_gatt_alloc(
        (void *)current_srv->characteristics,
        (hk_gatt_setup_info->chr_index + 1) * sizeof(hk_ble_chr_t));
    hk_ble_chr_t *chr =
        (hk_ble_chr_t *)&current_srv->characteristics[hk_gatt_setup_info->chr_index];

    memset(chr, 0, sizeof(hk_ble_chr_t));

    return chr;
}

void hk_gatt_chr_init(
    hk_ble_chr_t *chr,
    const ble_uuid_t *srv_uuid,
    ble_uuid128_t *chr_uuid,
    ble_gatt_chr_flags flags,
    hk_session_t *session,
    bool add_descriptors)
{
    chr->uuid = &chr_uuid->u;
    chr->access_cb = hk_gatt_access_callback;
    chr->flags = flags;
    chr->arg = (void *)session;

    size_t memory_size = 3 * sizeof(hk_ble_descriptor_t);
    chr->descriptors = (hk_ble_descriptor_t *)malloc(memory_size);
    memset((void *)chr->descriptors, 0, memory_size);

    chr->descriptors[0].uuid = &hk_uuid_manager_desciptor_instance_id.u;
    chr->descriptors[0].att_flags = BLE_ATT_F_READ;
    chr->descriptors[0].arg = (void *)session;
    chr->descriptors[0].access_cb = hk_gatt_access_callback;

    // chr->descriptors[1].uuid = &hk_uuid_manager_descriptor_format.u;
    // chr->descriptors[1].att_flags = BLE_ATT_F_READ;
    // chr->descriptors[1].arg = (void *)session;
    // chr->descriptors[1].access_cb = hk_gatt_access_callback;
}

void hk_gatt_init()
{
    HK_LOGD("Initializing GATT.");

    hk_gatt_setup_info = malloc(sizeof(hk_session_setup_info_t));
    hk_gatt_setup_info->srv_index = -1;
    hk_gatt_setup_info->chr_index = -1;
    hk_gatt_setup_info->instance_id = 0;
}

void hk_gatt_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden)
{
    if (hk_gatt_setup_info->srv_index >= 0)
    {
        // add end marker to chr array of last srv
        hk_ble_srv_t *last_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
        hk_gatt_alloc_new_chr(last_srv);
        hk_gatt_setup_info->chr_index = -1;
    }

    hk_ble_srv_t *srv = hk_gatt_alloc_new_srv();

    ble_uuid128_t *srv_uuid = hk_uuid_manager_get((uint8_t)srv_type);
    srv->type = 1;
    srv->uuid = &srv_uuid->u;

    // add srv id chr
    hk_ble_chr_t *chr = hk_gatt_alloc_new_chr(srv); // todo: combine in one function
    hk_session_t *session = (hk_session_t *)malloc(sizeof(hk_session_t));
    session->static_data = NULL;
    session->srv_id = hk_gatt_setup_info->srv_id = session->chr_index = hk_gatt_setup_info->instance_id++;
    hk_gatt_chr_init(chr, srv->uuid, (ble_uuid128_t *)&hk_uuid_manager_srv_id, BLE_GATT_CHR_F_READ, session, false);
}

void *hk_gatt_add_chr(
    hk_chr_types_t chr_type,
    void (*read)(hk_mem *response),
    void (*write)(hk_mem *request, hk_mem *response),
    bool can_notify,
    int16_t min_length,
    int16_t max_length)
{
    hk_ble_srv_t *current_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    ble_uuid128_t *chr_uuid = hk_uuid_manager_get((uint8_t)chr_type);
    hk_ble_chr_t *chr = hk_gatt_alloc_new_chr(current_srv);

    hk_session_t *session = hk_session_create(chr_type, hk_gatt_setup_info);
    session->srv_uuid = (ble_uuid128_t *)current_srv->uuid;
    session->read_callback = read;
    session->write_callback = write;
    session->max_length = max_length;
    session->min_length = min_length;

    ble_gatt_chr_flags flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ;

    if (can_notify)
    {
        flags |= BLE_GATT_CHR_F_INDICATE;
    }

    hk_gatt_chr_init(
        chr,
        current_srv->uuid,
        chr_uuid,
        flags,
        session, true);
    return NULL;
}

void hk_gatt_add_chr_static_read(hk_chr_types_t chr_type, const char *value)
{
    hk_ble_srv_t *current_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    ble_uuid128_t *chr_uuid = hk_uuid_manager_get((uint8_t)chr_type);
    hk_ble_chr_t *chr = hk_gatt_alloc_new_chr(current_srv);

    hk_session_t *session = hk_session_create(chr_type, hk_gatt_setup_info);
    session->srv_uuid = (ble_uuid128_t *)current_srv->uuid;
    session->static_data = value;

    hk_gatt_chr_init(
        chr,
        current_srv->uuid,
        chr_uuid,
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ,
        session, true);
}

void hk_gatt_end_config()
{
    if (hk_gatt_setup_info->srv_index >= 0)
    {
        // add end marker to chr array of last srv
        hk_ble_srv_t *last_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
        hk_gatt_alloc_new_chr(last_srv);
    }

    // add end marker to srvs array
    hk_gatt_alloc_new_srv();

    free(hk_gatt_setup_info);
}

void hk_gatt_start()
{
    HK_LOGD("Starting GATT.");
    ble_svc_gatt_init();

    int rc = ble_gatts_count_cfg(hk_gatt_srvs);
    if (rc != 0)
    {
        HK_LOGE("gatt_svr_init ble_gatts_count_cfg: %d", rc);
        //return rc;
    }

    rc = ble_gatts_add_svcs(hk_gatt_srvs);
    if (rc != 0)
    {
        HK_LOGE("Error setting gatt config: %d", rc);
    }
}