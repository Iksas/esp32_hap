/**
 * @file hk_global_state.h
 *
 * Functions work with and access the global state.
 */

#pragma once

#include <stdio.h>

/**
 * @brief Initialize global state
 *
 * Initialize global state.
 */
void hk_global_state_init();

/**
 * @brief Get the global state
 *
 * Returns the global state.
 * 
 * @return Returns the global state.
 */
uint16_t hk_global_state_get();

/**
 * @brief Increases the global state
 *
 * Increases the global state.
 */
void hk_global_state_next();

/**
 * @brief Resets the global state
 *
 * Resets the global state.
 */
void hk_global_state_reset();