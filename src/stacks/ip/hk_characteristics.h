#pragma once

#include "../../utils/hk_mem.h"
#include "hk_session.h"

#include <stdio.h>
#include <esp_err.h>

void hk_characteristics_get(hk_session_t *session_context);
void hk_characteristics_put(hk_session_t *session_context);
void hk_characteristics_identify(hk_session_t *session_context);
void hk_characteristics_notify(void *characteristic);