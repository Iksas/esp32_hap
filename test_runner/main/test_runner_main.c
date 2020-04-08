#include <stdio.h>
#include <unity.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_system.h"

// static void unity_task(void *pvParameters)
// {
//     printf("Test runner task started!\n");
//     vTaskDelay(2); /* Delay a bit to let the main task be deleted */
//     printf("Running menu!\n");
//     unity_run_menu(); /* Doesn't return */
// }

void app_main(void)
{
    printf("Starting test runner task!\n");
    unity_run_menu(); /* Doesn't return */

    // // Note: if unpinning this task, change the way run times are calculated in
    // // unity_port_esp32.c
    // //xTaskCreatePinnedToCore(unity_task, "unityTask", 8192, NULL, 5, NULL, 0);
}

// /* setUp runs before every test */
// void setUp(void)
// {
//     printf("%s", ""); /* sneakily lazy-allocate the reent structure for this test task */
//     printf("setup!\n");
// }

// /* tearDown runs after every test */
// void tearDown(void)
// {
//     /* some FreeRTOS stuff is cleaned up by idle task */
//     vTaskDelay(5);
//     printf("tearDown!\n");
// }

