/**
 * @file hk_mem.h
 *
 * Functions for memory handling.
 */

#pragma once

#include <stdlib.h>
#include <stdbool.h>

/**
 * @brief A memory
 *
 * Type for holding memory information.
 */
typedef struct
{
    size_t size;
    char *ptr;
} hk_mem;

/**
 * @brief Allocate memory
 *
 * This will allocate a new memory holding the data and its length.
 *
 * @return Returns a pointer to the allocated memory.
 */
hk_mem *hk_mem_init();

/**
 * @brief Appends memory
 *
 * Appends memory to an existing one.
 *
 * @param mem A pointer to an initialized memory.
 * @param mem_to_append A pointer to an initialized memory, which has to be appended.
 */
void hk_mem_append(hk_mem *mem, hk_mem *mem_to_append);

/**
 * @brief Appends a memory buffer
 *
 * Appends a void buffer to the existing memory.
 *
 * @param mem A pointer to an initialized memory.
 * @param data A pointer to an initialized buffer.
 * @param size The length of the buffer.
 */
void hk_mem_append_buffer(hk_mem *mem, void *data, size_t size);

/**
 * @brief Appends a string
 *
 * Appends a string to the existing memory. The string terminator is not appended.
 *
 * @param mem A pointer to an initialized memory.
 * @param string A constant string.
 */
void hk_mem_append_string(hk_mem *mem, const char *string);

/**
 * @brief Appends a string terminator
 *
 * Appends a string terminator to the existing memory.
 *
 * @param mem A pointer to an initialized memory.
 */
void hk_mem_append_string_terminator(hk_mem *mem);

/**
 * @brief Reallocates the memory
 *
 * Reallocate the memory to the given length.
 *
 * @param mem A pointer to an initialized memory.
 * @param size The new length of the memory.
 */
void hk_mem_set(hk_mem *mem, size_t size);

/**
 * @brief Replaces the memory
 *
 * Replaces the memory with the given one.
 *
 * @param mem A pointer to an initialized memory.
 * @param mem_to_set The new data.
 */
void hk_mem_set_mem(hk_mem *mem, hk_mem* mem_to_set);

/**
 * @brief Free the memory
 *
 * Free the memory.
 */
void hk_mem_free(hk_mem *mem);

/**
 * @brief Compare the memory
 *
 * Compares two memories against each other.
 *
 * @param mem1 A pointer to an initialized memory.
 * @param mem2 A pointer to an initialized memory.
 * 
 * @return True if equal, otherwise false.
 */
bool hk_mem_equal(hk_mem *mem1, hk_mem *mem2);

/**
 * @brief Compare memory agains string
 *
 * Compares the memory agains a string.
 *
 * @param mem A pointer to an initialized memory.
 * @param string A constant string.
 * 
 * @return True if equal, otherwise false.
 */
bool hk_mem_equal_str(hk_mem *mem, const char *str);

/**
 * @brief Get string from memory.
 *
 * Returns a string constructed from the memory area. Be sure to free the string, 
 * when it is not used anymore
 *
 * @param mem A pointer to an initialized memory.
 * 
 * @return The string.
 */
char *hk_mem_to_string(hk_mem *mem);

/**
 * @brief Get debug string from memory.
 *
 * Returns a string constructed from the memory area. Be sure to free the string, 
 * when it is not used anymore
 *
 * @param mem A pointer to an initialized memory.
 * 
 * @return The string.
 */
char* hk_mem_to_debug_string(hk_mem *mem);
