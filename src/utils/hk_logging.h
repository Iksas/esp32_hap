#pragma once

#include <esp_log.h>
#include <esp_err.h>
#include <stdlib.h>
#include <string.h>

#define __FILENAME__ (strstr(__FILE__, "/hk_") ? strstr(__FILE__, "/hk_") + 1 : __FILE__) // write reverse strstr and use / instead of /hk_
#define TAG  "hap"
#define HK_LOGV(message, ...) ESP_LOGV(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 
#define HK_LOGD(message, ...) ESP_LOGD(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 
#define HK_LOGI(message, ...) ESP_LOGI(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 
#define HK_LOGW(message, ...) ESP_LOGW(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 
#define HK_LOGE(message, ...) ESP_LOGE(TAG, "%s(%d)-> " message, __FILENAME__, __LINE__, ##__VA_ARGS__) 
#define HK_LOGEE(error) ESP_LOGE(TAG, "%s", esp_err_to_name(error))

void hk_log_print_bytewise(const char *title, char *data, size_t size, bool formatted);
void hk_log_print_as_string(const char *title, char *data, size_t size);