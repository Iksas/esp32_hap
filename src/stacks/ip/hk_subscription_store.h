/**
 * @file hk_fascade.h
 *
 * A fascade to make it easier to create simple homekit devices.
 */

#pragma once

#include "hk_accessories_store.h"
#include "hk_session.h"

/**
 * @brief Adds a subscription for the characteristic for the socket.
 *
 * Adds a subscription for the given characteristic/socket pair.
 *
 * @param chr The characteristic to add the subscription for.
 * @param socket The socket to add the characteristic for.
 */
esp_err_t hk_subscription_store_add(hk_chr_t *chr, int socket);

/**
 * @brief Returns all sockets the given characteristic has a subscription for.
 *
 * This function is used to send a notification for a given characteristic.
 *
 * @param chr The characteristic the sockets are requested for.
 * @param sockets The sockets the characteristic has subscriptions for.
 * @param number_of_sockets The number of sockets returned.
 */
esp_err_t hk_subscription_store_get(hk_chr_t *chr, int **sockets, size_t *number_of_sockets);

/**
 * @brief Removes a subscription for the characteristic for the socket.
 *
 * Removes a subscription for the given characteristic/socket pair.
 *
 * @param chr The characteristic to add the subscription for.
 * @param socket The socket to add the characteristic for.
 */
esp_err_t hk_subscription_store_remove_all(int socket);

/**
 * @brief Adds a subscription for the characteristic for the socket.
 *
 * Adds a subscription for the given characteristic/socket pair.
 *
 * @param chr The characteristic to add the subscription for.
 * @param socket The socket to add the characteristic for.
 */
esp_err_t hk_subscription_store_remove(hk_chr_t *chr, int socket);

/**
 * @brief Frees all ressources used by this module.
 */
void hk_subscription_store_free();