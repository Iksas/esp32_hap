#pragma once

#include "hk_accessories_store.h"
#include "hk_session.h"

void hk_subscription_store_add_session(hk_chr_t *chr, hk_session_t *session);
hk_session_t** hk_subscription_store_get_sessions(hk_chr_t *chr);
void hk_subscription_store_remove_session(hk_session_t *session);
void hk_subscription_store_remove_session_from_subscription(hk_chr_t *chr, hk_session_t *session);
void hk_subscription_store_free();