#include "hk_server_transport.h"

#include <sys/socket.h>
#include <math.h>

#include "../../crypto/hk_chacha20poly1305.h"
#include "../../utils/hk_logging.h"
#include "../../utils/hk_util.h"
#include "hk_session.h"

#define HK_MAX_RECV_SIZE 1024 // refer to spec 6.5.2
#define HK_AAD_SIZE 2
#define HK_AUTHTAG_SIZE 16 //16 = CHACHA20_POLY1305_AUTH_TAG_LENGTH
#define HK_MAX_DATA_SIZE HK_MAX_RECV_SIZE - HK_AAD_SIZE - HK_AUTHTAG_SIZE

typedef struct hk_server_transport_context
{
    char *received_buffer;
    size_t received_submitted_length;
    size_t received_length;
    size_t received_frame_count;
    size_t sent_frame_count;
    bool is_secure;
} hk_server_transport_context_t;

static void hk_server_on_free_session_ctx(void *ctx)
{
    hk_session_t *session = (hk_session_t *)ctx;
    hk_session_free(session);
}

static void hk_server_transport_context_free(hk_server_transport_context_t *context)
{
    free(context->received_buffer);
    free(context);
}

static void hk_server_on_free_session_transport_ctx(void *ctx)
{
    HK_LOGD("Freeing transport ctx.");
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)ctx;
    hk_server_transport_context_free(transport_context);
}

static int hk_server_transport_sock_err(const char *ctx, int sockfd)
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
            return HTTPD_SOCK_ERR_FAIL;
        }
        else
        {
            offset_out += message_size;
        }

        offset_in += message_size + HK_AAD_SIZE + HK_AUTHTAG_SIZE;
    }

    return offset_out;
}

static int hk_server_transport_recv(httpd_handle_t handle, int socket, char *buffer, size_t buffer_length, int flags)
{
    HK_LOGD("%d - hk_server_transport_recv (%d)", socket, buffer_length);
    int ret = 0;
    (void)handle;
    if (buffer == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    // getting contexts
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(handle, socket);
    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(handle, socket);

    if (transport_context->is_secure)
    {
        // we have to workaround the standard http server behaviour a little bit. Normally the server tries to catch
        // only chunks of data. But we have to receive the whole message to decrypt it. Therefore, we cache the received
        // and decrypted message and provide it block by block to the server.

        size_t size_to_submit_from_buffer = transport_context->received_length - transport_context->received_submitted_length;
        if (size_to_submit_from_buffer < 1)
        {
            // recv with max package size. Refer to spec 6.5.2
            char buffer_recv[HK_MAX_RECV_SIZE];
            ret = recv(socket, buffer_recv, HK_MAX_RECV_SIZE, flags);
            if (ret < 0)
            {
                return hk_server_transport_sock_err("recv", socket);
            }

            // decrypt the received message into transport context buffer
            memset(transport_context->received_buffer, 0, HK_MAX_RECV_SIZE);
            transport_context->received_submitted_length = 0;
            size_to_submit_from_buffer = transport_context->received_length = 
                ret = hk_server_transport_decrypt(transport_context, session->keys, buffer_recv, transport_context->received_buffer, ret);

            if (ret < 0)
            {
                HK_LOGE("%d - Could not pre process received data.", socket);
                return HTTPD_SOCK_ERR_FAIL;
            }
        }

        // find max length to submit to server and copy it into buffer
        size_t copy_length = MIN(buffer_length, size_to_submit_from_buffer);
        memcpy(buffer, transport_context->received_buffer + transport_context->received_submitted_length, copy_length);

        // set offset for next block and returned data length
        transport_context->received_submitted_length += copy_length;
        ret = copy_length;
    }
    else
    {
        ret = recv(socket, buffer, buffer_length, flags);
        if (ret < 0)
        {
            return hk_server_transport_sock_err("recv", socket);
        }
    }

    HK_LOGD("%d - Received: \n%s", socket, buffer);
    return ret;
}

static int hk_server_transport_encrypt_and_send(int socket, hk_server_transport_context_t *context, hk_conn_key_store_t *keys, const char *in, size_t in_length, int flags)
{
    char nonce[12] = {
        0,
    };
    size_t pending_size = in_length;
    char *pending = (char *)in;
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
        if (ret != ESP_OK)
        {
            return HTTPD_SOCK_ERR_FAIL;
        }

        ret = send(socket, encrypted, encrypted_size, flags);
        if (ret < 0)
        {
            return ret;
        }

        pending += chunk_size;
    }

    return in_length;
}

static int hk_server_transport_send(httpd_handle_t handle, int socket, const char *buffer, size_t buffer_length, int flags)
{
    HK_LOGD("%d - hk_server_transport_send", socket);
    char content[buffer_length + 1];
    memcpy(content, buffer, buffer_length);
    content[buffer_length] = '\0';
    HK_LOGD("%d - Sending: \n%s", socket, content);

    int ret = 0;
    (void)handle;
    if (buffer == NULL)
    {
        return HTTPD_SOCK_ERR_INVALID;
    }

    // getting contexts
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(handle, socket);
    hk_session_t *session = (hk_session_t *)httpd_sess_get_ctx(handle, socket);

    if (transport_context->is_secure)
    {
        ret = hk_server_transport_encrypt_and_send(socket, transport_context, session->keys, buffer, buffer_length, flags);
    }
    else
    {
        ret = send(socket, buffer, buffer_length, flags);
        if (ret < 0)
        {
            return hk_server_transport_sock_err("send", socket);
        }
    }

    HK_LOGD("%d - Result: %d", socket, ret);
    return ret;
}

static hk_server_transport_context_t *hk_server_transport_context_init()
{
    hk_server_transport_context_t *context = (hk_server_transport_context_t *)malloc(sizeof(hk_server_transport_context_t));

    context->sent_frame_count = 0;
    context->received_frame_count = 0;
    context->received_submitted_length = 0;
    context->received_length = 0;
    context->received_buffer = (char *)malloc(HK_MAX_RECV_SIZE);
    context->is_secure = false;

    return context;
}

esp_err_t hk_server_transport_on_open_connection(httpd_handle_t handle, int socket)
{
    HK_LOGD("%d - Connection open", socket);

    // setting session context
    hk_session_t *session = hk_session_init(socket);
    httpd_sess_set_ctx(handle, socket, (void *)session, hk_server_on_free_session_ctx);

    // setting transport context
    hk_server_transport_context_t *transport_context = hk_server_transport_context_init(socket);
    httpd_sess_set_transport_ctx(handle, socket, (void *)transport_context, hk_server_on_free_session_transport_ctx);

    // setting recv/send overrides to handle encryption
    httpd_sess_set_recv_override(handle, socket, hk_server_transport_recv);
    httpd_sess_set_send_override(handle, socket, hk_server_transport_send);

    return ESP_OK;
}

esp_err_t hk_server_transport_set_session_secure(httpd_handle_t handle, int socket)
{
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(handle, socket);
    transport_context->is_secure = true;
    return ESP_OK;
}