/**
 * @file hk_pairings.h
 *
 * Logic to execute a pairing request.
 */

#pragma once

#include <esp_err.h>

#include "../include/hk_mem.h"

/**
 * @brief Executes pairings request
 *
 * Executes pairings request.
 *
 * @param request The request send from the controller.
 * @param request The response to be sent to the controller.
 * @param kill_session Sets a flag, if the connection has to be killed.
 * @param is_paired Sets the paired status.
 */
esp_err_t hk_pairings(hk_mem *request, hk_mem *response, bool *kill_session, bool *is_paired);
