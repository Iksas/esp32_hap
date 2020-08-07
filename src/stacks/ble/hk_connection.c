#include "hk_connection.h"

#include <esp_timer.h>
#include <esp_system.h>

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"
#include "hk_uuids.h"

hk_connection_t *hk_connection_connections = NULL;

hk_transaction_t *hk_connection_transaction_get_by_uuid(hk_connection_t *connection, const ble_uuid128_t *chr_uuid)
{
    hk_transaction_t *transaction_to_return = NULL;
    hk_ll_foreach(connection->transactions, transaction)
    {
        if (hk_uuids_cmp(transaction->chr_uuid, chr_uuid))
        {
            transaction_to_return = transaction;
            //todo: break;
        }
    }

    if (transaction_to_return == NULL)
    {
        char char_uid_str[40];
        hk_uuids_to_name(chr_uuid, char_uid_str);
        HK_LOGW("%d - Requested transaction was not found: %s", connection->handle, char_uid_str);
    }

    return transaction_to_return;
}

hk_transaction_t *hk_connection_transaction_init(hk_connection_t *connection, uint8_t transaction_id, uint8_t opcode, const ble_uuid128_t *chr_uuid)
{
    char char_uid_str[40];
    hk_uuids_to_name(chr_uuid, char_uid_str);
    HK_LOGV("%d.%d - Adding new transaction for %s.", connection->handle, transaction_id, char_uid_str);

    hk_transaction_t *transaction = connection->transactions = hk_ll_init(connection->transactions);

    transaction->id = transaction_id;
    transaction->opcode = opcode;
    transaction->chr_uuid = chr_uuid;

    transaction->request = hk_mem_init();
    transaction->expected_request_length = 0;

    transaction->response = hk_mem_init();
    transaction->response_sent = 0;
    transaction->response_status = 0;
    transaction->start_time = esp_timer_get_time();

    return transaction;
}

void hk_connection_transaction_free(hk_connection_t *connection, hk_transaction_t *transaction)
{
    transaction->id = -1;
    hk_mem_free(transaction->request);
    hk_mem_free(transaction->response);

    connection->transactions = hk_ll_remove(connection->transactions, transaction);
    HK_LOGV("%d.%d - Transaction closed.", connection->handle, transaction->id);
}

hk_connection_t *hk_connection_get_by_handle(uint16_t handle)
{
    hk_ll_foreach(hk_connection_connections, connection)
    {
        if (connection->handle == handle)
        {
            return connection;
        }
    }

    HK_LOGW("%d - Requested connection was not found.", handle);
    return NULL;
}

hk_connection_t *hk_connection_get_all()
{
    return hk_connection_connections;
}

hk_connection_t *hk_connection_init(uint16_t handle, hk_mem *address)
{
    HK_LOGD("%d - Adding new connection (%s).", handle, address->ptr);

    hk_connection_t *connection = hk_connection_connections = hk_ll_init(hk_connection_connections);

    connection->handle = handle;
    connection->is_secure = false;
    connection->global_state_was_changed_once = false;
    connection->received_frame_count = 0;
    connection->sent_frame_count = 0;
    connection->device_id = hk_mem_init();
    connection->security_keys = hk_conn_key_store_init();
    connection->transactions = NULL;
    connection->mtu_size = (uint8_t)256;

    return connection;
}

void hk_connection_mtu_set(uint16_t handle, uint16_t mtu_size)
{
    HK_LOGV("%d - Setting new mtu to connection: %d", handle, mtu_size);
    hk_connection_t *connection = hk_connection_get_by_handle(handle);
    connection->mtu_size = mtu_size;
}

void hk_connection_free(uint16_t handle)
{
    HK_LOGV("%d - Removing connection from %d connections.", handle, hk_ll_count(hk_connection_connections));
    hk_connection_t *connection = hk_connection_get_by_handle(handle);

    while (connection->transactions != NULL)
    {
        hk_connection_transaction_free(connection, connection->transactions);
    }

    hk_ll_free(connection->transactions);

    connection->handle = -1;
    hk_mem_free(connection->device_id);
    hk_conn_key_store_free(connection->security_keys);

    hk_connection_connections = hk_ll_remove(hk_connection_connections, connection);
    HK_LOGD("%d - Connection closed.", handle);
}