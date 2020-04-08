#include "hk_pair_verify_keys.h"

hk_pair_verify_keys_t *hk_pair_verify_keys_init()
{
    hk_pair_verify_keys_t *keys = (hk_pair_verify_keys_t *)malloc(sizeof(hk_pair_verify_keys_t));
    keys->response_key = hk_mem_init();
    keys->request_key = hk_mem_init();
    keys->shared_secret = hk_mem_init();

    return keys;
}

void hk_pair_verify_keys_free(hk_pair_verify_keys_t *keys)
{
    hk_mem_free(keys->request_key);
    hk_mem_free(keys->response_key);
    hk_mem_free(keys->shared_secret);
    free(keys);
}

void hk_pair_verify_keys_reset(hk_pair_verify_keys_t *keys)
{
    hk_mem_set(keys->request_key, 0);
    hk_mem_set(keys->response_key, 0);
    hk_mem_set(keys->shared_secret, 0);
}