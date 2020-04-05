
#pragma once

#include "../../../include/hk_mem.h" 

#include "../hk_connection.h"
#include "../hk_chr.h"

esp_err_t hk_srv_signature_read(hk_transaction_t *transaction, hk_chr_t *chr);
