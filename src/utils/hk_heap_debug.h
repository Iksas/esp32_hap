/**
 * @file hk_heap_debug.h
 *
 * Functions for debugging memory leaks on heap.
 */

#pragma once

/**
 * @brief Starts heap debugging.
 *
 * Starts heap debugging.
 */
void hk_heap_debug_start();

/**
 * @brief Resumes heap debugging.
 *
 * Resumes heap debugging.
 */
void hk_heap_debug_resume();

/**
 * @brief Pauses heap debugging.
 *
 * Pauses heap debugging.
 */
void hk_heap_debug_pause();

/**
 * @brief Stops heap debugging.
 *
 * Stops heap debugging.
 */
void hk_heap_debug_stop();