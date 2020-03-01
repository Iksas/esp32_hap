#include "hk_session.h"

hk_session_t *hk_session_create(hk_chr_types_t chr_type, hk_session_setup_info_t *setup_info)
{
    hk_session_t * session = (hk_session_t *)malloc(sizeof(hk_session_t));

    if(setup_info != NULL){
        session->srv_index = setup_info->srv_index;
        session->srv_id = setup_info->srv_id;
        session->chr_index = setup_info->instance_id++;
    }

    session->chr_type = chr_type;
    session->static_data = NULL;
    session->transaction_id = -1;
    session->last_opcode = -1;
    session->request_length = -1;
    session->max_length = -1;
    session->min_length = -1;
    session->read_callback = NULL;
    session->write_callback = NULL;
    session->request = hk_mem_create();

    return session;
}