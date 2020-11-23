#pragma once

#include <stddef.h>

/**
 * @file hk_ll.h
 *
 * A linked list library.
 */

/**
 * @brief Initalize a list.
 *
 * Allocates the first item in the list.
 *
 * @param ll A pointer to the first item of the list.
 * 
 * @return Returns a pointer to the new list.
 */
#define hk_ll_init(ll) (typeof (ll)) _hk_ll_init(ll, sizeof(*ll))

/**
 * @brief Iterates to the next item in the list.
 *
 * Looks for the next item in the list.
 *
 * @param ll The pointer to the list.
 * 
 * @return Returns a pointer in the list, which points to the next item.
 */
#define hk_ll_next(ll) (typeof (ll)) _hk_ll_next(ll)


/**
 * @brief Iterates throgh the list.
 *
 * Returns a pointer to all items in the list until all are throuth. Use
 * it like: hk_ll_foreach(list_ptr, item_ptr)
 * 
 * Attention: Be careful when deleting in the list. You have to restore 
 * the pointers:
 * 
 * hk_ll_foreach(ll, item)
 * {
 *     ...
 *     ll = hk_ll_remove(ll, item);
 *     ...
 * }
 *
 * @param ll A pointer to the list.
 * @param item A pointer to the current list item.
 */
#define hk_ll_foreach(ll, item) \
    for (typeof (ll) item = ll; item; item = _hk_ll_next(item))
#define hk_ll_break() break


/**
 * @brief Initalize a list.
 *
 * Allocates the first item in the list.
 *
 * @param ll A pointer to the first item of the list.
 * @param size The size of one item.
 * 
 * @return Returns a pointer to the new list.
 */
void *_hk_ll_init(void *ll, size_t size);

/**
 * @brief Iterates to the next item in the list.
 *
 * Looks for the next item in the list.
 *
 * @param ll The pointer to the list.
 * 
 * @return Returns a pointer in the list, which points to the next item.
 */
void *_hk_ll_next(void *ll);

/**
 * @brief Frees all ressources of the list.
 *
 * Frees all ressources of the list.
 *
 * @param ll The pointer to the list.
 */
void hk_ll_free(void *ll);

/**
 * @brief Reverses the list.
 *
 * After calling this method, all items in the list are iterated
 * in reversed sequence.
 *
 * @param ll The pointer to the list.
 * 
 * @return Returns the new pointer to the list.
 */
void * hk_ll_reverse(void *ll);

/**
 * @brief Removes one item from the list.
 *
 * Removes one item from the list.
 *
 * @param ll The pointer to the list.
 * 
 * @return Returns the new pointer to the list.
 */
void *hk_ll_remove(void *ll, void *ll_to_remove);

/**
 * @brief Returns the number of items in the list.
 *
 * Counts the items in the list and returns the number.
 *
 * @param ll The pointer to the list.
 * 
 * @return Returns the number of items in the list.
 */
int hk_ll_count(void *ll);
