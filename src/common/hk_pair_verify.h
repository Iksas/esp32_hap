/**
 * @file hk_pair_verify.h
 *
 * Logic to execute pair verify
 */

#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"
#include "hk_conn_key_store.h"

/**
 * @brief Executes pair verify
 *
 * Executes pair verify.
 *
 * @param request The request send from the controller.
 * @param request The response to be sent to the controller.
 * @param keys The keys of the connection.
 * @param is_session_encrypted Sets the encrypted status.
 */
esp_err_t hk_pair_verify(hk_mem *request, hk_mem *result, hk_conn_key_store_t *keys, bool *is_session_encrypted);
