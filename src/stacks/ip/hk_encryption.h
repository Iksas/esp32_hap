#pragma once

#include "../../utils/hk_mem.h"
#include "hk_session.h"

#include <esp_err.h>

esp_err_t hk_encryption_preprocessor(hk_session_t *session, hk_mem *in, hk_mem *out);
esp_err_t hk_encryption_postprocessor(hk_session_t *session, hk_mem *in);