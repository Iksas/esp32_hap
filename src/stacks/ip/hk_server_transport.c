#include "hk_server_transport.h"

#include <sys/socket.h>

#include "../../crypto/hk_chacha20poly1305.h"
#include "../../utils/hk_logging.h"
#include "hk_session.h"

#define HK_AAD_SIZE 2
#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define HK_MAX_DATA_SIZE 1024 - HK_AAD_SIZE - HK_AUTHTAG_SIZE

static int hk_server_sock_err(const char *ctx, int sockfd)
{
    int errval;
    HK_LOGW("error in %s : %d", ctx, errno);

    switch (errno)
    {
    case EAGAIN:
    case EINTR:
        errval = HTTPD_SOCK_ERR_TIMEOUT;
        break;
    case EINVAL:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        errval = HTTPD_SOCK_ERR_INVALID;
        break;
    default:
        errval = HTTPD_SOCK_ERR_FAIL;
    }
    return errval;
}


static int hk_server_transport_decrypt(hk_server_transport_context_t *context, hk_conn_key_store_t *keys, char *in, char *out, size_t length)
{
    size_t offset_in = 0;
    size_t offset_out = 0;

    while (offset_in < length)
    {
        char *encrypted = in + offset_in;
        size_t message_size = encrypted[0] + encrypted[1] * 256;
        char nonce[12] = {
            0,
        };
        nonce[4] = context->received_frame_count % 256;
        nonce[5] = context->received_frame_count++ / 256;

        esp_err_t ret = hk_chacha20poly1305_decrypt_buffer(
            keys->request_key, nonce, encrypted, HK_AAD_SIZE, encrypted + HK_AAD_SIZE, out + offset_out, message_size);

        if (ret)
        {
            return -1;
        }
        else
        {
            offset_out += message_size;
        }

        offset_in += message_size + HK_AAD_SIZE + HK_AUTHTAG_SIZE;
    }

    return offset_out;
}

int hk_server_transport_recv(httpd_handle_t handle, int socket, char *buffer, size_t buffer_length, int flags)
{
    HK_LOGD("hk_server_transport_recv");
    int ret = 0;
    (void)handle;
    if (buffer == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    HK_LOGD("Getting contexts.");
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(handle, socket);
    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(handle, socket);

    HK_LOGD("Checking if session is secure.");
    if (transport_context->is_secure)
    {
        char buffer_recv[buffer_length];

        ret = recv(socket, buffer_recv, buffer_length, flags);
        if (ret < 0)
        {
            return hk_server_sock_err("recv", socket);
        }

        ret = hk_server_transport_decrypt(transport_context, session->keys, buffer_recv, buffer, ret);
        if (ret < 0)
        {
            HK_LOGE("Could not pre process received data of socket %d.", socket);
            return HTTPD_SOCK_ERR_FAIL;
        }
    }
    else
    {
        ret = recv(socket, buffer, buffer_length, flags);
        if (ret < 0)
        {
            return hk_server_sock_err("recv", socket);
        }
    }

    HK_LOGD("Received: \n%s", buffer);
    return ret;
}

int hk_server_send(httpd_handle_t handle, int socket, const char *buf, size_t buf_len, int flags)
{
    (void)handle;
    if (buf == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    int ret = send(socket, buf, buf_len, flags);
    if (ret < 0)
    {
        return hk_server_sock_err("send", socket);
    }

    //hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(handle, socket);
    return ret;
}

esp_err_t hk_server_transport_encrypt(hk_server_transport_context_t *context, hk_conn_key_store_t *keys, hk_mem *in,
                                      esp_err_t (*callback)(hk_mem *frame_data, void *args), void *args)
{
    char nonce[12] = {
        0,
    };
    size_t pending_size = in->size;
    char *pending = in->ptr;
    while (pending_size > 0)
    {
        size_t chunk_size = pending_size < HK_MAX_DATA_SIZE ? pending_size : HK_MAX_DATA_SIZE;
        pending_size -= chunk_size;
        size_t encrypted_size = HK_AAD_SIZE + chunk_size + HK_AUTHTAG_SIZE;
        char encrypted[encrypted_size];
        encrypted[0] = chunk_size % 256;
        encrypted[1] = chunk_size / 256;

        nonce[4] = context->sent_frame_count % 256;
        nonce[5] = context->sent_frame_count++ / 256;

        esp_err_t ret = hk_chacha20poly1305_encrypt_buffer(keys->response_key, nonce, encrypted, HK_AAD_SIZE,
                                                           pending, encrypted + HK_AAD_SIZE, chunk_size);
        if (ret < 0)
        {
            return ret;
        }
        hk_mem *data_to_send = hk_mem_init(); // is disposed by server, after it was sent
        hk_mem_append_buffer(data_to_send, encrypted, encrypted_size);
        ret = callback(data_to_send, args);
        if (ret < 0)
        {
            return ret;
        }

        pending += chunk_size;
    }

    return ESP_OK;
}

hk_server_transport_context_t *hk_server_transport_context_init()
{
    hk_server_transport_context_t *context = (hk_server_transport_context_t *)malloc(sizeof(hk_server_transport_context_t));

    context->sent_frame_count = 0;
    context->received_frame_count = 0;
    context->is_secure = false;

    return context;
}

void hk_server_transport_context_free(hk_server_transport_context_t *context)
{
    free(context);
}