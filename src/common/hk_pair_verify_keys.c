#include "hk_pair_verify_keys.h"

hk_pair_verify_keys_t *hk_pair_verify_keys_init()
{
    hk_pair_verify_keys_t *keys = (hk_pair_verify_keys_t *)malloc(sizeof(hk_pair_verify_keys_t));
    keys->response_key = hk_mem_create();
    keys->request_key = hk_mem_create();

    return keys;
}

void hk_pair_verify_keys_free(hk_pair_verify_keys_t *keys)
{
    hk_mem_free(keys->request_key);
    hk_mem_free(keys->response_key);
    free(keys);
}