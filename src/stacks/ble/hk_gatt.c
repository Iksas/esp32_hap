#include "hk_gatt.h"

#include <host/ble_uuid.h>
#include <host/ble_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include <host/ble_hs.h>

#include "../../utils/hk_logging.h"

#include "hk_session.h"
#include "hk_uuid_manager.h"
#include "hk_session_security.h"
#include "hk_advertising.h"
#include "operations/hk_chr_signature_read.h"
#include "operations/hk_chr_write.h"
#include "operations/hk_chr_read.h"
#include "operations/hk_chr_timed_write.h"
#include "operations/hk_srv_signature_read.h"
#include "operations/hk_chr_configuration.h"
#include "operations/hk_protocol_configuration.h"

#include "../../crypto/hk_chacha20poly1305.h"

typedef struct ble_gatt_svc_def hk_ble_srv_t;
typedef struct ble_gatt_chr_def hk_ble_chr_t;
typedef struct ble_gatt_dsc_def hk_ble_descriptor_t;

hk_session_setup_info_t *hk_gatt_setup_info = NULL;
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
    hk_logu("Read descriptor of chr", chr_uuid);
    const ble_uuid128_t *descriptor_uuid = BLE_UUID128(ctxt->dsc->uuid);
    hk_logu("with id", descriptor_uuid);
    hk_session_t *session = (hk_session_t *)arg;

    if (hk_gatt_cmp(descriptor_uuid, (ble_uuid128_t *)&hk_uuid_manager_descriptor_instance_id))
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

static int hk_gatt_write_ble_chr(uint16_t connection_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    esp_err_t res = ESP_OK;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_session_t *session = (hk_session_t *)arg;
    session->connection_handle = connection_handle;
    session->response_status = 0;

    uint8_t buffer_len = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t buffer[buffer_len];
    uint16_t out_len = 0;
    rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
    hk_mem *received = hk_mem_create();
    const ble_uuid128_t *chr_uuid_pair_verify = hk_uuid_manager_get((uint8_t)HK_CHR_PAIR_VERIFY);
    if (hk_session_security_is_secured_get() && !hk_gatt_cmp(chr_uuid, chr_uuid_pair_verify))
    {
        hk_mem *received_before_encryption = hk_mem_create();
        hk_mem_append_buffer(received_before_encryption, buffer, out_len);
        hk_session_security_decrypt(
            received_before_encryption,
            received);
        hk_mem_free(received_before_encryption);
    }
    else
    {
        hk_mem_append_buffer(received, buffer, out_len);
    }

    uint8_t control_field = received->ptr[0];
    if (control_field && 0b01000000) // according specification bit 7 (zero based) should be set to 1. But it isnt????
    {
        // continuation
        hk_mem_append_buffer(session->request, received->ptr + 2, out_len - 2); //continuation is preceeded by controlfield and transaction id
        hk_mem_log("Received continuation", received);
        HK_LOGD("Received continuation %d of %d.", session->request->size, session->request_length);
    }
    else
    {
        printf("\n");
        hk_logu("Received new request", chr_uuid);
        hk_mem_log("Received new request data", received);
        session->last_opcode = received->ptr[1];
        HK_LOGD("Set op code to %x", session->last_opcode);
        session->transaction_id = received->ptr[2];
        HK_LOGD("Set transaction id to %x", session->transaction_id);

        hk_mem_set(session->request, 0);
        hk_mem_set(session->response, 0);
        if (received->size > 5)
        {
            session->request_length = received->ptr[5] + received->ptr[6] * 256;
            hk_mem_append_buffer(session->request, received->ptr + 7, received->size - 7); // -7 because of PDU start
        }
        else
        {
            session->request_length = 0;
        }
    }

    if (session->request_length == session->request->size)
    {
        hk_mem_set(session->response, 0);
        session->response_sent = 0;
        switch (session->last_opcode)
        {
        case 1:
            HK_LOGD("Request complete, executing signature read.");
            res = hk_chr_signature_read_response(chr_uuid, session);
            break;
        case 2:
            HK_LOGD("Request complete, executing characteristic write.");
            res = hk_chr_write_response(chr_uuid, session);
            break;
        case 3:
            HK_LOGD("Request complete, executing characteristic read.");
            res = hk_chr_read_response(chr_uuid, session);
            break;
        case 4:
            HK_LOGD("Request complete, executing characteristic timed write.");
            res = hk_chr_timed_write_response(chr_uuid, session);
            break;
        case 5:
            HK_LOGD("Request complete, executing characteristic execute write.");
            res = hk_chr_execute_write_response(chr_uuid, session);
            break;
        case 6:
            HK_LOGD("Request complete, executing signature read.");
            res = hk_srv_signature_read_response(chr_uuid, session);
            break;
        case 7:
            HK_LOGD("Request complete, executing characteristic configuration.");
            res = hk_chr_configuration_response(chr_uuid, session);
            break;
        case 8:
            HK_LOGD("Request complete, executing protocol configuration.");
            res = hk_protocol_configuration_response(chr_uuid, session);
            break;
        default:
            HK_LOGE("Unknown opcode.");
            res = ESP_ERR_NOT_SUPPORTED;
        }
    }
    else
    {
        HK_LOGD("Received %d of %d. Waiting for continuation of fragmentation.", session->request->size, session->request_length);
    }

    hk_mem_free(received);

    if (res == ESP_ERR_HK_TERMINATE)
    {
        hk_advertising_terminate_connection(connection_handle);
    }
    else if (res != ESP_OK)
    {
        session->response_status = 0x06;
    }
    else
    {
        session->response_status = 0x00;
    }

    if (session->response_status != 0x00)
    {
        HK_LOGE("Setting response status to %d.", session->response_status);
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
            uint8_t status = session->response_status; // status is zero, success
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

        const ble_uuid128_t *chr_uuid_pair_verify = hk_uuid_manager_get((uint8_t)HK_CHR_PAIR_VERIFY);
        if (hk_session_security_is_secured_get() && !hk_gatt_cmp(chr_uuid, chr_uuid_pair_verify))
        {
            hk_mem *encrypted_response = hk_mem_create();
            hk_session_security_encrypt(
                response,
                encrypted_response);
            rc = os_mbuf_append(ctxt->om, encrypted_response->ptr, encrypted_response->size);
            hk_mem_free(encrypted_response);
        }
        else
        {
            rc = os_mbuf_append(ctxt->om, response->ptr, response->size);
        }

        if (session->response_sent == session->response->size)
        { //this should not be needed, but homkit controller keeps asking even if everything was sent. Mybe a problem with fragmentation?
            session->response_sent = 0;
        }

        hk_mem_free(response);
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_access_callback(uint16_t connection_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        HK_LOGV("BLE_GATT_ACCESS_OP_READ_CHR");
        rc = hk_gatt_read_ble_chr(ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        HK_LOGV("BLE_GATT_ACCESS_OP_WRITE_CHR");
        rc = hk_gatt_write_ble_chr(connection_handle, ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC)
    {
        HK_LOGI("BLE_GATT_ACCESS_OP_READ_DSC");
        rc = hk_gatt_read_ble_descriptor(ctxt, arg);
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
    bool add_descriptors, // todo: remove
    bool can_notify)
{
    chr->uuid = &chr_uuid->u;
    chr->access_cb = hk_gatt_access_callback;
    chr->flags = flags;
    chr->arg = (void *)session;

    uint8_t number_of_descriptors = 2; // one for instance id and one for array end marker
    if (can_notify)
    {
        number_of_descriptors++;
    }

    size_t memory_size = number_of_descriptors * sizeof(hk_ble_descriptor_t);
    chr->descriptors = (hk_ble_descriptor_t *)malloc(memory_size);
    memset((void *)chr->descriptors, 0, memory_size); // set everything to zero. Especially important for array end marker

    chr->descriptors[0].uuid = &hk_uuid_manager_descriptor_instance_id.u;
    chr->descriptors[0].att_flags = BLE_ATT_F_READ;
    chr->descriptors[0].arg = (void *)session;
    chr->descriptors[0].access_cb = hk_gatt_access_callback;

    if (can_notify)
    {
        chr->descriptors[1].uuid = &hk_uuid_manager_descriptor_client_configuration.u;
        chr->descriptors[1].att_flags = BLE_ATT_F_READ;
        chr->descriptors[1].arg = (void *)session;
        chr->descriptors[1].access_cb = hk_gatt_access_callback;
    }
}

void hk_gatt_init()
{
    HK_LOGD("Initializing GATT.");

    hk_gatt_setup_info = malloc(sizeof(hk_session_setup_info_t));
    hk_gatt_setup_info->srv_index = -1;
    hk_gatt_setup_info->chr_index = -1;
    hk_gatt_setup_info->instance_id = 1;
}

void hk_gatt_add_srv(hk_srv_types_t srv_type, bool primary, bool hidden,
                     bool supports_configuration)
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
    session->srv_primary = hk_gatt_setup_info->srv_primary = primary;
    session->srv_hidden = hk_gatt_setup_info->srv_hidden = hidden;
    session->srv_supports_configuration = hk_gatt_setup_info->srv_supports_configuration = supports_configuration;
    hk_gatt_chr_init(chr, srv->uuid, (ble_uuid128_t *)&hk_uuid_manager_srv_id, BLE_GATT_CHR_F_READ, session, false, false);
}

void *hk_gatt_add_chr(
    hk_chr_types_t chr_type,
    esp_err_t (*read)(hk_mem *response),
    esp_err_t (*write)(hk_mem *request, hk_mem *response),
    bool can_notify,
    float min_length,
    float max_length)
{
    hk_ble_srv_t *current_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    ble_uuid128_t *chr_uuid = hk_uuid_manager_get((uint8_t)chr_type);
    hk_ble_chr_t *chr = hk_gatt_alloc_new_chr(current_srv);

    hk_session_t *session = hk_session_init(chr_type, hk_gatt_setup_info);
    session->srv_uuid = (ble_uuid128_t *)current_srv->uuid;
    session->srv_id = hk_gatt_setup_info->srv_id;
    session->srv_primary = hk_gatt_setup_info->srv_primary;
    session->srv_hidden = hk_gatt_setup_info->srv_hidden;
    session->srv_supports_configuration = hk_gatt_setup_info->srv_supports_configuration;

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
        session, true, can_notify);

    return NULL;
}

void hk_gatt_add_chr_static_read(hk_chr_types_t chr_type, const char *value)
{
    hk_ble_srv_t *current_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    ble_uuid128_t *chr_uuid = hk_uuid_manager_get((uint8_t)chr_type);
    hk_ble_chr_t *chr = hk_gatt_alloc_new_chr(current_srv);

    hk_session_t *session = hk_session_init(chr_type, hk_gatt_setup_info);
    session->srv_uuid = (ble_uuid128_t *)current_srv->uuid;
    session->srv_id = hk_gatt_setup_info->srv_id;
    session->srv_primary = hk_gatt_setup_info->srv_primary;
    session->srv_hidden = hk_gatt_setup_info->srv_hidden;
    session->srv_supports_configuration = hk_gatt_setup_info->srv_supports_configuration;

    session->static_data = value;

    hk_gatt_chr_init(
        chr,
        current_srv->uuid,
        chr_uuid,
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ,
        session, true, false);
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
        HK_LOGE("Error initializing services: %d", rc);
        //return rc;
    }

    rc = ble_gatts_add_svcs(hk_gatt_srvs);
    if (rc != 0)
    {
        HK_LOGE("Error setting gatt config: %d", rc);
    }
}