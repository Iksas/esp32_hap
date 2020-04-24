#include "hk_conn_key_store.h"

hk_conn_key_store_t *hk_conn_key_store_init()
{
    hk_conn_key_store_t *keys = (hk_conn_key_store_t *)malloc(sizeof(hk_conn_key_store_t));
    keys->response_key = hk_mem_init();
    keys->request_key = hk_mem_init();
    keys->shared_secret = hk_mem_init();

    return keys;
}

void hk_conn_key_store_free(hk_conn_key_store_t *keys)
{
    hk_mem_free(keys->request_key);
    hk_mem_free(keys->response_key);
    hk_mem_free(keys->shared_secret);
    free(keys);
}

void hk_conn_key_store_reset(hk_conn_key_store_t *keys)
{
    hk_mem_set(keys->request_key, 0);
    hk_mem_set(keys->response_key, 0);
    hk_mem_set(keys->shared_secret, 0);
}