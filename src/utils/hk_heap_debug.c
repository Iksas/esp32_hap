#include "hk_heap_debug.h"

#include "esp_heap_trace.h"

#define NUM_RECORDS 100
static heap_trace_record_t hk_heap_debug_trace_record[NUM_RECORDS];
static uint8_t hk_heap_debug_is_initialized = 0;

void hk_heap_debug_start()
{
    if (hk_heap_debug_is_initialized != 1)
    {
        ESP_ERROR_CHECK(heap_trace_init_standalone(hk_heap_debug_trace_record, NUM_RECORDS));
        hk_heap_debug_is_initialized = 1;
    }

    ESP_ERROR_CHECK(heap_trace_start(HEAP_TRACE_LEAKS));
}

void hk_heap_debug_resume()
{
    ESP_ERROR_CHECK(heap_trace_resume());
}

void hk_heap_debug_pause()
{
    ESP_ERROR_CHECK(heap_trace_stop());
}

void hk_heap_debug_stop()
{
    ESP_ERROR_CHECK(heap_trace_stop());
    heap_trace_dump();
}