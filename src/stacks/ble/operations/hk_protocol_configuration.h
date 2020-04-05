
#pragma once

#include "../../../include/hk_mem.h" 
#include "../../../common/hk_pair_verify_keys.h"

#include "../hk_connection.h"
#include "../hk_chr.h"

esp_err_t hk_protocol_configuration(hk_pair_verify_keys_t *keys, hk_transaction_t *transaction, hk_chr_t *chr);
