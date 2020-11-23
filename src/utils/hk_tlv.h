#pragma once

#include <stdlib.h>
#include <esp_types.h>
#include <esp_err.h>

#include "../include/hk_mem.h"

/**
 * @file hk_tlv.h
 *
 * Functions to work with type length value types.
 */

typedef struct
{
    char type;
    char length;
    char *value;
} hk_tlv_t;

/**
 * @brief Adds a string tlv.
 *
 * Adds a string tlv.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * @param data The null terminated string.
 * 
 * @return Returns the pointer to the tlv list.
 */
hk_tlv_t *hk_tlv_add_str(hk_tlv_t *tlv_list, char type, char *data);

/**
 * @brief Add a memory buffer to the tlv list.
 *
 * Add a memory buffer to the tlv list.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * @param data A pointer to the memory.
 * @param length The length of the memory.
 * 
 * @return Returns the pointer to the tlv.
 */
hk_tlv_t *hk_tlv_add_buffer(hk_tlv_t *tlv_list, char type, char *data, size_t length);

/**
 * @brief Add a memory buffer to the tlv list.
 *
 * Add a memory buffer to the tlv list.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * @param mem A pointer to the buffer.
 * 
 * @return Returns the pointer to the tlv.
 */
hk_tlv_t *hk_tlv_add_mem(hk_tlv_t *tlv_list, char type, hk_mem* mem);

/**
 * @brief Adds an uint8_t the tlv list.
 *
 * Adds an uint8_t the tlv list.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * @param data The uint8_t.
 * 
 * @return Returns the pointer to the tlv.
 */
hk_tlv_t *hk_tlv_add_uint8(hk_tlv_t *tlv_list, char type, uint8_t data);

/**
 * @brief Adds an uint16_t the tlv list.
 *
 * Adds an uint16_t the tlv list.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * @param data The uint16_t.
 * 
 * @return Returns the pointer to the tlv.
 */
hk_tlv_t *hk_tlv_add_uint16(hk_tlv_t *tlv_list, char type, uint16_t data);

/**
 * @brief Returns the a buffer stored for the type.
 *
 * Looks in the list for a tlv with the given type and returns the corresponding memory.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * @param result The resulting buffer.
 * 
 * @return Returns the pointer to the tlv.
 */
esp_err_t hk_tlv_get_mem_by_type(hk_tlv_t *tlv, char type, hk_mem *result);

/**
 * @brief Returns a tlv of the list with the given type.
 *
 * Looks in the list for a tlv with the given type and retuns it.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param type The type of the tlv.
 * 
 * @return Returns the pointer to the tlv.
 */
hk_tlv_t *hk_tlv_get_tlv_by_type(hk_tlv_t *tlv_list, char type);

/**
 * @brief Counts the tlv items in the list.
 *
 * Iterates through all items and returns the count.
 *
 * @param tlv_list A pointer to the tlv list.
 * 
 * @return Returns the number of items.
 */
size_t hk_tlv_get_size(hk_tlv_t *tlv_list);

/**
 * @brief Frees the given tlv list.
 *
 * Frees the given tlv list.
 *
 * @param tlv_list A pointer to the tlv list.
 */
void hk_tlv_free(hk_tlv_t *tlv_list);

/**
 * @brief Serializes the given tlv list.
 *
 * Serializes the given tlv list into a buffer.
 *
 * @param tlv_list A pointer to the tlv list.
 * @param result The resulting buffer.
 */
void hk_tlv_serialize(hk_tlv_t *tlv_list, hk_mem *result);

/**
 * @brief Deserializes a list from the given buffer.
 *
 * Deserializes a list from the given buffer.
 *
 * @param data The buffer to deserialize.
 * 
 * @return Returns the pointer to the tlv list.
 */
hk_tlv_t *hk_tlv_deserialize(hk_mem *data);

/**
 * @brief Logs the given tlv.
 *
 * Logs the given tlv
 *
 * @param title A message to write.
 * @param tlv_list A pointer to the tlv list.
 * @param with_value True in order to log the values, too.
 * @param formatted Adds tabs and newlines.
 */
void hk_tlv_log(const char *title, hk_tlv_t *tlv_list, bool with_value, bool formatted);
