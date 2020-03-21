#include "hk_com.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <lwip/sockets.h>
#include <esp_err.h>

#include "../../utils/hk_ll.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_logging_lwip.h"
#include "../../utils/hk_math.h"
#include "hk_subscription_store.h"
#include "hk_session_security.h"

typedef struct
{
    int port;
    esp_err_t (*receiver)(hk_session_t *connection, hk_mem *data);
} hk_com_arguments_t;

esp_err_t hk_com_queue_frame(hk_mem *frame_data, void *args)
{
    hk_session_t *connection = (hk_session_t *)args;
    if (xQueueSendToBack(connection->response->data_to_send, &frame_data, 10) != pdPASS)
    {
        HK_LOGE("Error queuing data to send.");
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t hk_com_send_data(hk_session_t *connection)
{
    esp_err_t ret;
    if (connection->is_secure)
    {
        ret = hk_session_security_encrypt_frames(
            connection->encryption_data,
            connection->keys,
            connection->response->data,
            hk_com_queue_frame,
            (void *)connection);
    }
    else
    {
        hk_mem *frame_data = hk_mem_create(); // is disposed by server, after it was sent
        hk_mem_append(frame_data, connection->response->data);
        ret = hk_com_queue_frame(frame_data, (void *)connection);
    }

    return ret;
}

esp_err_t hk_com_open_connection(hk_session_t **connections, int listen_socket, fd_set *active_fds)
{
    struct sockaddr_in remote_addr;
    int len = sizeof(remote_addr);
    // accespt and get new socket file descriptor
    int socket = accept(listen_socket, (struct sockaddr *)&remote_addr, (socklen_t *)&len);

    if (socket < 0)
    {
        // happens sometimes, after removing device
        HK_LOGD("Could not accept new connection: %d", socket);
        hk_log_lwip_err(0, "Error accepting");

        return ESP_FAIL;
    }

    const struct timeval rcvtimeout = {10, 0}; /* 10 second timeout */
    setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &rcvtimeout, sizeof(rcvtimeout));

    const int yes = 1; /* enable sending keepalive probes for socket */
    setsockopt(socket, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(yes));

    const int idle = 180; /* 180 sec iddle before start sending probes */
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));

    const int interval = 30; /* 30 sec between probes */
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));

    const int maxpkt = 4; /* Drop connection after 4 probes without response */
    setsockopt(socket, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(maxpkt));

    *connections = hk_ll_new(*connections);
    hk_session_t *connection = *connections;
    hk_session_init(connection, socket);

    FD_SET(connection->socket, active_fds);

    HK_LOGD("%d - Opened connection from '%s'.", socket, inet_ntoa(remote_addr.sin_addr.s_addr));
    return ESP_OK;
}

void hk_com_handle_receive(hk_session_t *connection, esp_err_t (*receiver)(hk_session_t *connection, hk_mem *data))
{
    int buffer_size = 1050; // use a value bigger than max package size specified by hap
    char buffer[buffer_size];
    int recv_size = lwip_read(connection->socket, buffer, buffer_size);
    if (recv_size == buffer_size)
    {
        connection->should_close = true;
        HK_LOGE("%d - Received package bigger than excepted: %d", connection->socket, recv_size);
    }
    else if (recv_size < 0)
    {
        connection->should_close = true;
        hk_log_lwip_err(connection->socket, "Error receiving data");
    }
    else if (recv_size == 0)
    {
        connection->should_close = true;
        HK_LOGV("%d - Mark connection to be closed.", connection->socket);
    }
    else
    {
        HK_LOGV("%d - Received %d bytes", connection->socket, recv_size);
        hk_mem *data = hk_mem_create();

        if (connection->is_secure)
        {
            hk_mem *encrypted_data = hk_mem_create();
            hk_mem_append_buffer(encrypted_data, buffer, recv_size);
            esp_err_t ret = hk_session_security_decrypt_frames(connection->encryption_data, connection->keys, encrypted_data, data);
            if (ret != ESP_OK)
            {
                HK_LOGE("Could not pre process received data of socket %d.", connection->socket);
            }
            hk_mem_free(encrypted_data);
        }
        else
        {
            hk_mem_append_buffer(data, buffer, recv_size);
        }

        if (receiver(connection, data) != ESP_OK)
        {
            HK_LOGE("%d - Could not process received data.", connection->socket);
        }

        hk_mem_free(data);
    }
}

void hk_com_handle_send_data(hk_session_t *connection)
{
    hk_mem *data_to_send;
    if (xQueueReceive(connection->response->data_to_send, &data_to_send, 0))
    {
        int result = lwip_write(connection->socket, data_to_send->ptr, data_to_send->size);
        if (result < 0)
        {
            HK_LOGE("%d - Error sending data.", connection->socket);
        }

        hk_mem_free(data_to_send);
    }
}

esp_err_t hk_com_start_listening(int *listen_socket, int port)
{
    struct sockaddr_in server_address;
    int socket = lwip_socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_address, '0', sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(port);

    if (lwip_bind(socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
    {
        HK_LOGE("Could not bind to socket.");
        lwip_close(socket);
        return ESP_FAIL;
    };

    if (lwip_listen(socket, 10) != 0)
    {
        HK_LOGE("Could not start listening to socket.");
        lwip_close(socket);
        return ESP_FAIL;
    }

    *listen_socket = socket;

    HK_LOGD("Listening to socket %d on port %d", socket, port);
    return ESP_OK;
}

void hk_com_sending_data(hk_session_t *connections)
{
    for (hk_session_t *connection = connections; connection != NULL; connection = hk_ll_next(connection))
    {
        hk_com_handle_send_data(connection);
    }
}

void hk_com_receiving_data(hk_session_t *connections, fd_set *read_fds, esp_err_t (*receiver)(hk_session_t *connection, hk_mem *data))
{
    // iterate through connections to receive data.
    for (hk_session_t *connection = connections; connection != NULL; connection = hk_ll_next(connection))
    {
        if (FD_ISSET(connection->socket, read_fds))
        {
            hk_com_handle_receive(connection, receiver);
        }
    }
}

void hk_com_mark_connections_to_be_closed(hk_session_t *connections)
{
    // Iterate over all connections to find one, that has to be killed. If found,
    // mark all connections to the same device as to be closed
    for (hk_session_t *c1 = connections; c1; c1 = hk_ll_next(c1))
    {
        if (c1->device_id != NULL)
        {
            if (c1->kill)
            {
                for (hk_session_t *c2 = connections; c2; c2 = hk_ll_next(c2))
                {
                    if (c2->device_id != NULL && c1->socket != c2->socket)
                    {
                        if (strcmp(c2->device_id, c1->device_id) == 0)
                        {
                            c2->should_close = true;
                        }
                    }
                }
            }
        }
    }
}

void hk_com_close_connections(hk_session_t **connections, fd_set *active_fds)
{
    for (hk_session_t *connection = *connections; connection != NULL;)
    {
        if (connection->should_close)
        {
            HK_LOGD("%d - Closing connection.", connection->socket);
            hk_subscription_store_remove_session(connection);

            FD_CLR(connection->socket, active_fds);
            HK_LOGD("Closing socket %d", connection->socket);
            close(connection->socket);

            hk_session_free(connection);

            hk_session_t *next = hk_ll_next(connection);
            *connections = hk_ll_remove(*connections, connection);
            connection = next;
        }
        else
        {
            connection = hk_ll_next(connection);
        }
    }
}

void hk_com_task(void *args_ptr)
{
    hk_com_arguments_t *args = (hk_com_arguments_t *)args_ptr;

    int listen_socket = 0;
    esp_err_t err = hk_com_start_listening(&listen_socket, args->port);
    if (err != ESP_OK)
    {
        return;
    }

    fd_set active_fds, read_fds;
    FD_ZERO(&active_fds);
    FD_SET(listen_socket, &active_fds);
    int highest_socket = listen_socket;
    int connection_count = 0;
    hk_session_t *connections = NULL;

    for (;;)
    {
        read_fds = active_fds;

        struct timeval timeout = {1, 0}; // 1 second timeout
        int triggered_fds = select(highest_socket + 1, &read_fds, NULL, NULL, &timeout);
        if (triggered_fds > 0) // more than zero sockets have data
        {
            // check if listener socket has data and if so, create new connection
            if (FD_ISSET(listen_socket, &read_fds))
            {
                if (connection_count >= 15)
                {
                    HK_LOGW("Could not accept new connection, because only 16 are allowed.");
                }
                else
                {
                    esp_err_t res = hk_com_open_connection(&connections, listen_socket, &active_fds);
                    if (res == ESP_OK)
                    {
                        highest_socket = MAX(highest_socket, connections->socket);
                    }
                }
            }

            hk_com_receiving_data(connections, &read_fds, args->receiver);
        }

        hk_com_sending_data(connections);
        hk_com_mark_connections_to_be_closed(connections);
        hk_com_close_connections(&connections, &active_fds);
    }
}

esp_err_t hk_com_start(int port, esp_err_t (*receiver)(hk_session_t *connection, hk_mem *data))
{
    hk_com_arguments_t *args = malloc(sizeof(hk_com_arguments_t));
    args->port = port;
    args->receiver = receiver;

    xTaskCreate(hk_com_task, "hk_com", 12288, args, tskIDLE_PRIORITY, NULL);

    return ESP_OK;
}