#pragma once

#include "../../utils/hk_mem.h"
#include "hk_session.h"

#include <stdio.h>
#include <esp_err.h>

void hk_chrs_get(hk_session_t *session_context);
void hk_chrs_put(hk_session_t *session_context);
void hk_chrs_identify(hk_session_t *session_context);
void hk_chrs_notify(void *chr);