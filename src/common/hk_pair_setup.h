#pragma once

#include <esp_err.h>
#include <stdlib.h>

#include "../include/hk_mem.h"
#include "../utils/hk_tlv.h"
#include "hk_conn_key_store.h"

/**
 * @brief Executes pair setup
 *
 * Executes pair setup.
 *
 * @param request The request send from the controller.
 * @param request The response to be sent to the controller.
 * @param keys The keys of the connection.
 * @param device_id The device id of the connection.
 * @param is_paired Sets the paired status.
 */
esp_err_t hk_pair_setup(hk_mem *request, hk_mem *response, hk_conn_key_store_t *keys, hk_mem *device_id, bool *is_paired);
