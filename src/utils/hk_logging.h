#pragma once

#include <esp_log.h>
#include <esp_err.h>
#include <stdlib.h>
#include <string.h>

/**
 * @file hk_ll.h
 *
 * A linked list library.
 */

#define __FILENAME__ (strstr(__FILE__, "/hk_") ? strstr(__FILE__, "/hk_") + 1 : __FILE__) // write reverse strstr and use / instead of /hk_
#define TAG  "hk"

/**
 * @brief Logs verbose message.
 */
#define HK_LOGV(message, ...) ESP_LOGV(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 

/**
 * @brief Logs debug message.
 */
#define HK_LOGD(message, ...) ESP_LOGD(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 

/**
 * @brief Logs info message.
 */
#define HK_LOGI(message, ...) ESP_LOGI(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 

/**
 * @brief Logs warning message.
 */
#define HK_LOGW(message, ...) ESP_LOGW(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 

/**
 * @brief Logs error message.
 */
#define HK_LOGE(message, ...) ESP_LOGE(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 

/**
 * @brief Logs an error message.
 */
#define HK_LOGEE(error) HK_LOGE("Error executing: %s (%d)", esp_err_to_name(error), error)


/**
 * @brief Logs the given string bytewise.
 */
void hk_log_print_bytewise(const char *title, char *data, size_t size, bool formatted);