#pragma once

#include <inttypes.h>
#include <host/ble_uuid.h>
#include "../../include/hk_mem.h"
#include "../../common/hk_pair_verify.h"

typedef struct
{
    uint8_t id;
    uint8_t opcode;
    const ble_uuid128_t *chr_uuid;

    hk_mem *request;
    uint16_t expected_request_length;

    hk_mem *response;
    uint16_t response_sent;
    uint8_t response_status;
} hk_transaction_t;

typedef struct
{
    uint16_t connection_handle;
    hk_pair_verify_keys_t *security_keys;
    bool is_secure;
    uint32_t received_frame_count;
    uint32_t sent_frame_count;
    hk_mem *device_id;
    hk_transaction_t *transactions;
} hk_connection_t;

hk_transaction_t *hk_connection_transaction_get_by_uuid(hk_connection_t *connection, const ble_uuid128_t *chr_uuid);
hk_transaction_t *hk_connection_transaction_init(hk_connection_t *connection, const ble_uuid128_t *chr_uuid);
void hk_connection_transaction_free(hk_connection_t *connection, hk_transaction_t *transaction);

hk_connection_t *hk_connection_get_by_handle(uint16_t connection_handle);
hk_connection_t *hk_connection_init(uint16_t connection_handle);
void hk_connection_free(uint16_t connection_handle);