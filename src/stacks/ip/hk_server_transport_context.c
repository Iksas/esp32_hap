#include "hk_server_transport_context.h"

#include "../../utils/hk_logging.h"

hk_server_transport_context_t *hk_server_transport_context_init()
{
    hk_server_transport_context_t *context = (hk_server_transport_context_t *)malloc(sizeof(hk_server_transport_context_t));

    context->sent_frame_count = 0;
    context->received_frame_count = 0;
    context->received_submitted_length = 0;
    context->received_length = 0;
    context->received_buffer = (char *)malloc(HK_MAX_RECV_SIZE);
    context->is_secure = false;

    context->keys = hk_conn_key_store_init();

    return context;
}

void hk_server_transport_context_free(void *context)
{
    HK_LOGD("Freeing transport context.");
    hk_server_transport_context_t *transport_context = (hk_server_transport_context_t *)context;

    hk_conn_key_store_free(transport_context->keys);

    free(transport_context->received_buffer);
    free(transport_context);
}

hk_server_transport_context_t *hk_server_transport_context_get(httpd_handle_t handle, int socket)
{
    return (hk_server_transport_context_t *)httpd_sess_get_transport_ctx(handle, socket);
}