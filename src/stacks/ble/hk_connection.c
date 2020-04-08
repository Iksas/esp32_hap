#include "hk_connection.h"

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
        HK_LOGW("Requested transaction was not found: %s", hk_uuids_to_str(chr_uuid));
    }

    return transaction_to_return;
}

hk_transaction_t *hk_connection_transaction_init(hk_connection_t *connection, const ble_uuid128_t *chr_uuid)
{
    HK_LOGV("Adding new transaction with id %s.", hk_uuids_to_str(chr_uuid));

    hk_transaction_t *transaction = connection->transactions = hk_ll_new(connection->transactions);

    transaction->id = -1;
    transaction->opcode = -1;
    transaction->chr_uuid = chr_uuid;
    
    transaction->request = hk_mem_create();
    transaction->expected_request_length = 0;

    transaction->response = hk_mem_create();
    transaction->response_sent = 0;
    transaction->response_status = 0;

    return transaction;
}

void hk_connection_transaction_free(hk_connection_t *connection, hk_transaction_t *transaction)
{
    HK_LOGV("Removing transaction %s from %d transactions.", hk_uuids_to_str(transaction->chr_uuid), hk_ll_count(connection->transactions));

    transaction->id = -1;
    hk_mem_free(transaction->request);
    hk_mem_free(transaction->response);

    connection->transactions = hk_ll_remove(connection->transactions, transaction);
}

hk_connection_t *hk_connection_get_by_handle(uint16_t connection_handle)
{
    hk_ll_foreach(hk_connection_connections, connection)
    {
        if (connection->connection_handle == connection_handle)
        {
            return connection;
        }
    }

    HK_LOGW("Requested connection was not found: %d", connection_handle);
    return NULL;
}

hk_connection_t *hk_connection_init(uint16_t connection_handle)
{
    HK_LOGV("Adding new connection with handle %d.", connection_handle);

    hk_connection_t *connection = hk_connection_connections = hk_ll_new(hk_connection_connections);

    connection->connection_handle = connection_handle;
    connection->is_secure = false;
    connection->received_frame_count = 0;
    connection->sent_frame_count = 0;
    connection->device_id = hk_mem_create();
    connection->security_keys = hk_pair_verify_keys_init();
    connection->transactions = NULL;

    return connection;
}

void hk_connection_free(uint16_t connection_handle)
{
    HK_LOGD("Removing connection with handle %d from %d connections.", connection_handle, hk_ll_count(hk_connection_connections));
    hk_connection_t *connection = hk_connection_get_by_handle(connection_handle);
    
    while (connection->transactions != NULL)
    {
        hk_connection_transaction_free(connection, connection->transactions);
    }

    hk_ll_free(connection->transactions);

    connection->connection_handle = -1;
    hk_mem_free(connection->device_id);
    hk_pair_verify_keys_free(connection->security_keys);

    hk_connection_connections = hk_ll_remove(hk_connection_connections, connection);
}