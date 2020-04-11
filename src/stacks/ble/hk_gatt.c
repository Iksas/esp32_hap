#include "hk_gatt.h"

#include <host/ble_uuid.h>
#include <host/ble_gatt.h>
#include <services/gap/ble_svc_gap.h>
#include <services/gatt/ble_svc_gatt.h>
#include <host/ble_hs.h>

#include "../../utils/hk_logging.h"

#include "hk_chr.h"
#include "hk_uuids.h"
#include "hk_connection_security.h"
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

hk_chr_setup_info_t *hk_gatt_setup_info = NULL;
hk_ble_srv_t *hk_gatt_srvs = NULL;
uint8_t last_transaction_id;

static int hk_gatt_read_ble_descriptor(struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    const ble_uuid128_t *descriptor_uuid = BLE_UUID128(ctxt->dsc->uuid);
    HK_LOGV("Read descriptor of chr %s with id %s", hk_uuids_to_str(chr_uuid), hk_uuids_to_str(descriptor_uuid));
    hk_chr_t *chr = (hk_chr_t *)arg;

    if (hk_uuids_cmp(descriptor_uuid, (ble_uuid128_t *)&hk_uuids_descriptor_instance_id))
    {
        HK_LOGV("Returning instance id for chr: %d", chr->chr_index);
        uint16_t id = chr->chr_index;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    }
    else
    {
        HK_LOGE("Operation not implemented.");
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int_fast8_t hk_gatt_decrypt(struct ble_gatt_access_ctxt *ctxt, const ble_uuid128_t *chr_uuid, hk_connection_t *connection, hk_mem *request)
{
    uint8_t buffer_len = OS_MBUF_PKTLEN(ctxt->om);
    uint8_t buffer[buffer_len];
    uint16_t out_len = 0;
    int rc = ble_hs_mbuf_to_flat(ctxt->om, buffer, buffer_len, &out_len);
    const ble_uuid128_t *chr_uuid_pair_verify = hk_uuids_get((uint8_t)HK_CHR_PAIR_VERIFY);
    if (connection->is_secure && !hk_uuids_cmp(chr_uuid, chr_uuid_pair_verify))
    {
        HK_LOGV("Decrypting request for %s", hk_uuids_to_str(chr_uuid));
        hk_mem *received_before_encryption = hk_mem_init();
        hk_mem_append_buffer(received_before_encryption, buffer, out_len);
        hk_connection_security_decrypt(connection, received_before_encryption, request); //todo: catch error
        hk_mem_free(received_before_encryption);
    }
    else
    {
        hk_mem_append_buffer(request, buffer, out_len);
    }

    return rc;
}

static int hk_gatt_encrypt(struct ble_gatt_access_ctxt *ctxt, const ble_uuid128_t *chr_uuid, hk_connection_t *connection, hk_mem *response)
{
    int rc;
    const ble_uuid128_t *chr_uuid_pair_verify = hk_uuids_get((uint8_t)HK_CHR_PAIR_VERIFY);
    if (connection->is_secure && !hk_uuids_cmp(chr_uuid, chr_uuid_pair_verify))
    {
        HK_LOGV("Encrypting response");
        hk_mem *encrypted_response = hk_mem_init();
        hk_connection_security_encrypt(connection, response, encrypted_response); //todo: catch error
        rc = os_mbuf_append(ctxt->om, encrypted_response->ptr, encrypted_response->size);
        hk_mem_free(encrypted_response);
    }
    else
    {
        rc = os_mbuf_append(ctxt->om, response->ptr, response->size);
    }

    return rc;
}

static int hk_gatt_write_ble_chr(uint16_t connection_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    esp_err_t res = ESP_OK;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_connection_t *connection = hk_connection_get_by_handle(connection_handle);
    hk_chr_t *chr = (hk_chr_t *)arg;

    hk_mem *request = hk_mem_init();
    rc = hk_gatt_decrypt(ctxt, chr_uuid, connection, request);

    hk_transaction_t *transaction;
    uint8_t control_field = request->ptr[0];
    bool continuation = control_field && 0b10000000;
    if (continuation)
    {
        // continuation
        transaction = hk_connection_transaction_get_by_uuid(connection, chr_uuid);
        hk_mem_append_buffer(transaction->request, request->ptr + 2, request->size - 2);
    }
    else
    {
        transaction = hk_connection_transaction_init(connection, chr_uuid);
        transaction->opcode = request->ptr[1];
        transaction->id = request->ptr[2];

        if (request->size > 5)
        {
            transaction->expected_request_length = request->ptr[5] + request->ptr[6] * 256;
            hk_mem_append_buffer(transaction->request, request->ptr + 7, request->size - 7); // -7 because of PDU start
        }
    }

    if (transaction->expected_request_length == transaction->request->size)
    {
        HK_MEM_ASSIGN_BYTE_STR(request_bytes, transaction->request);
        HK_UUIDS_ASSIGN_NAME(uuid_name, chr_uuid);
        switch (transaction->opcode)
        {
        case 1:
            HK_LOGV("Signature read for %s with: %s", uuid_name, request_bytes);
            res = hk_chr_signature_read(chr_uuid, transaction, chr);
            break;
        case 2:
            HK_LOGD("Characteristic write for %s with: %s", uuid_name, request_bytes);
            res = hk_chr_write(connection, transaction, chr);
            break;
        case 3:
            HK_LOGD("Characteristic read for %s with: %s", uuid_name, request_bytes);
            res = hk_chr_read(transaction, chr);
            break;
        case 4:
            HK_LOGD("Characteristic timed write for %s with: %s", uuid_name, request_bytes);
            res = hk_chr_timed_write(transaction, chr);
            break;
        case 5:
            HK_LOGD("Characteristic execute write for %s with: %s", uuid_name, request_bytes);
            res = hk_chr_execute_write(connection, transaction, chr);
            break;
        case 6:
            HK_LOGV("Signature read for %s with: %s", uuid_name, request_bytes);
            res = hk_srv_signature_read(transaction, chr);
            break;
        case 7:
            HK_LOGD("Characteristic configuration for %s with: %s", uuid_name, request_bytes);
            res = hk_chr_configuration(transaction);
            break;
        case 8:
            HK_LOGD("Protocol configuration for %s with: %s", uuid_name, request_bytes);
            res = hk_protocol_configuration(connection->security_keys, transaction, chr);
            break;
        default:
            HK_LOGE("Unknown opcode.");
            res = ESP_ERR_NOT_SUPPORTED;
        }
    }
    else
    {
        HK_LOGD("Received %d of %d. Waiting for continuation of fragmentation.", transaction->request->size, transaction->expected_request_length);
    }

    if (res == ESP_ERR_HK_TERMINATE)
    {
        hk_advertising_terminate_connection(connection_handle);
    }
    else if (res != ESP_OK)
    {
        transaction->response_status = 0x06;
    }
    else
    {
        transaction->response_status = 0x00;
    }

    if (transaction->response_status != 0x00)
    {
        HK_LOGE("Setting response status to %d.", transaction->response_status);
    }

    rc = rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
    HK_LOGV("Done, status is %d, %d.", transaction->response_status, rc);

    return rc;
}

static int hk_gatt_read_ble_chr(uint16_t connection_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    const ble_uuid128_t *chr_uuid = BLE_UUID128(ctxt->chr->uuid);
    hk_chr_t *chr = (hk_chr_t *)arg;

    if (hk_uuids_cmp(chr_uuid, (ble_uuid128_t *)&hk_uuids_srv_id))
    {
        HK_LOGV("Received new request to read ble characteristic %s and returning instance id of service %d.",
                hk_uuids_to_str(chr_uuid), chr->srv_id);
        uint16_t id = chr->srv_id;
        rc = os_mbuf_append(ctxt->om, &id, sizeof(uint16_t));
    }
    else
    {
        HK_LOGV("Received new request to read ble characteristic %s", hk_uuids_to_str(chr_uuid));
        hk_connection_t *connection = hk_connection_get_by_handle(connection_handle);
        hk_mem *response = hk_mem_init();
        hk_transaction_t *transaction = hk_connection_transaction_get_by_uuid(connection, chr_uuid);
        if (transaction == NULL)
        {
            return 0;
        }
        bool continuation = transaction->response_sent > 0;
        bool has_body = transaction->response->size > 0;
        uint8_t control_field = continuation ? 0b10000010 : 0b00000010;
        hk_mem_append_buffer(response, (char *)&control_field, 1);
        hk_mem_append_buffer(response, (char *)&transaction->id, 1);

        if (!continuation) // no continuation
        {
            hk_mem_append_buffer(response, (char *)&transaction->response_status, 1);
        }

        if (!continuation && has_body)
        {
            hk_mem_append_buffer(response, (char *)&transaction->response->size, 2);
        }

        if (has_body)
        {
            size_t response_size = transaction->response->size - transaction->response_sent;
            if (response_size > 246)
            {
                response_size = 246;
            }

            hk_mem_append_buffer(response, transaction->response->ptr + transaction->response_sent, response_size);
            transaction->response_sent += response_size;
        }

        //hk_log_print_bytewise("Sending ble read response", response->ptr, response->size, false);
        rc = hk_gatt_encrypt(ctxt, chr_uuid, connection, response);

        if (transaction->response_sent == transaction->response->size)
        {
            hk_connection_transaction_free(connection, transaction);
        }

        hk_mem_free(response);
        HK_LOGV("Sent response.");
    }

    return rc == 0 ? 0 : BLE_ATT_ERR_INSUFFICIENT_RES;
}

static int hk_gatt_access_callback(uint16_t connection_handle, uint16_t attr_handle, struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    int rc = 0;
    if (ctxt->op == BLE_GATT_ACCESS_OP_READ_CHR)
    {
        HK_LOGV("BLE_GATT_ACCESS_OP_READ_CHR");
        rc = hk_gatt_read_ble_chr(connection_handle, ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR)
    {
        HK_LOGV("BLE_GATT_ACCESS_OP_WRITE_CHR");
        rc = hk_gatt_write_ble_chr(connection_handle, ctxt, arg);
    }
    else if (ctxt->op == BLE_GATT_ACCESS_OP_READ_DSC)
    {
        HK_LOGV("BLE_GATT_ACCESS_OP_READ_DSC");
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
    hk_ble_chr_t *ble_chr,
    const ble_uuid_t *srv_uuid,
    ble_uuid128_t *chr_uuid,
    ble_gatt_chr_flags flags,
    hk_chr_t *chr,
    bool add_descriptors, // todo: remove
    bool can_notify)
{
    ble_chr->uuid = &chr_uuid->u;
    ble_chr->access_cb = hk_gatt_access_callback;
    ble_chr->flags = flags;
    ble_chr->arg = (void *)chr;

    uint8_t number_of_descriptors = 2; // one for instance id and one for array end marker
    if (can_notify)
    {
        number_of_descriptors++;
    }

    size_t memory_size = number_of_descriptors * sizeof(hk_ble_descriptor_t);
    ble_chr->descriptors = (hk_ble_descriptor_t *)malloc(memory_size);
    memset((void *)ble_chr->descriptors, 0, memory_size); // set everything to zero. Especially important for array end marker

    ble_chr->descriptors[0].uuid = &hk_uuids_descriptor_instance_id.u;
    ble_chr->descriptors[0].att_flags = BLE_ATT_F_READ;
    ble_chr->descriptors[0].arg = (void *)chr;
    ble_chr->descriptors[0].access_cb = hk_gatt_access_callback;

    if (can_notify)
    {
        ble_chr->descriptors[1].uuid = &hk_uuids_descriptor_client_configuration.u;
        ble_chr->descriptors[1].att_flags = BLE_ATT_F_READ;
        ble_chr->descriptors[1].arg = (void *)chr;
        ble_chr->descriptors[1].access_cb = hk_gatt_access_callback;
    }
}

void hk_gatt_init()
{
    HK_LOGD("Initializing GATT.");

    hk_gatt_setup_info = malloc(sizeof(hk_chr_setup_info_t));
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

    ble_uuid128_t *srv_uuid = hk_uuids_get((uint8_t)srv_type);
    srv->type = 1;
    srv->uuid = &srv_uuid->u;

    // add srv id chr
    hk_ble_chr_t *ble_chr = hk_gatt_alloc_new_chr(srv); // todo: combine in one function
    hk_chr_t *chr = (hk_chr_t *)malloc(sizeof(hk_chr_t));
    chr->static_data = NULL;
    chr->srv_id = hk_gatt_setup_info->srv_id = chr->chr_index = hk_gatt_setup_info->instance_id++;
    chr->srv_primary = hk_gatt_setup_info->srv_primary = primary;
    chr->srv_hidden = hk_gatt_setup_info->srv_hidden = hidden;
    chr->srv_supports_configuration = hk_gatt_setup_info->srv_supports_configuration = supports_configuration;
    hk_gatt_chr_init(ble_chr, srv->uuid, (ble_uuid128_t *)&hk_uuids_srv_id, BLE_GATT_CHR_F_READ, chr, false, false);
}

void *hk_gatt_add_chr(
    hk_chr_types_t chr_type,
    esp_err_t (*read)(hk_mem *response),
    esp_err_t (*write)(hk_mem *request),
    esp_err_t (*write_with_response)(hk_connection_t *connection, hk_mem *request, hk_mem *response),
    bool can_notify,
    float min_length,
    float max_length)
{
    hk_ble_srv_t *current_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    ble_uuid128_t *chr_uuid = hk_uuids_get((uint8_t)chr_type);
    hk_ble_chr_t *ble_chr = hk_gatt_alloc_new_chr(current_srv);

    hk_chr_t *chr = hk_chr_init(chr_type, hk_gatt_setup_info);
    chr->srv_uuid = (ble_uuid128_t *)current_srv->uuid;
    chr->srv_id = hk_gatt_setup_info->srv_id;
    chr->srv_primary = hk_gatt_setup_info->srv_primary;
    chr->srv_hidden = hk_gatt_setup_info->srv_hidden;
    chr->srv_supports_configuration = hk_gatt_setup_info->srv_supports_configuration;

    chr->read_callback = read;
    chr->write_callback = write;
    chr->write_with_response_callback = write_with_response;
    chr->max_length = max_length;
    chr->min_length = min_length;

    ble_gatt_chr_flags flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ;

    if (can_notify)
    {
        flags |= BLE_GATT_CHR_F_INDICATE;
    }

    hk_gatt_chr_init(
        ble_chr,
        current_srv->uuid,
        chr_uuid,
        flags,
        chr, true, can_notify);

    return NULL;
}

void hk_gatt_add_chr_static_read(hk_chr_types_t chr_type, const char *value)
{
    hk_ble_srv_t *current_srv = &hk_gatt_srvs[hk_gatt_setup_info->srv_index];
    ble_uuid128_t *chr_uuid = hk_uuids_get((uint8_t)chr_type);
    hk_ble_chr_t *ble_chr = hk_gatt_alloc_new_chr(current_srv);

    hk_chr_t *chr = hk_chr_init(chr_type, hk_gatt_setup_info);
    chr->srv_uuid = (ble_uuid128_t *)current_srv->uuid;
    chr->srv_id = hk_gatt_setup_info->srv_id;
    chr->srv_primary = hk_gatt_setup_info->srv_primary;
    chr->srv_hidden = hk_gatt_setup_info->srv_hidden;
    chr->srv_supports_configuration = hk_gatt_setup_info->srv_supports_configuration;

    chr->static_data = value;

    hk_gatt_chr_init(
        ble_chr,
        current_srv->uuid,
        chr_uuid,
        BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_WRITE | BLE_GATT_CHR_PROP_READ,
        chr, true, false);
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