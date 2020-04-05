#include "hk_chr.h"

hk_chr_t *hk_chr_init(hk_chr_types_t chr_type, hk_chr_setup_info_t *setup_info)
{
    hk_chr_t * chr = (hk_chr_t *)malloc(sizeof(hk_chr_t));

    if(setup_info != NULL){
        chr->srv_index = setup_info->srv_index;
        chr->srv_id = setup_info->srv_id;
        chr->chr_index = setup_info->instance_id++;
    }
    chr->srv_primary = false;// todo: put this stuff in an service object
    chr->srv_hidden = false;
    chr->srv_supports_configuration = false;
    chr->chr_type = chr_type;
    chr->static_data = NULL;
    chr->max_length = 0;
    chr->min_length = 0;
    chr->read_callback = NULL;
    chr->write_callback = NULL;
    chr->write_with_response_callback = NULL;

    return chr;
}