#define LOGNAME "SISWI"

#include <hk.h>
#include <hk_fascade.h>

#include <esp_system.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#include "freertos/task.h"

#define LED 19
uint8_t air_quality = 2; // values between 0 and 5 are supported.
void *chr_air_quality_ptr = NULL;

void identify()
{
    ESP_LOGI(LOGNAME, "Identify");
}


esp_err_t air_quality_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&air_quality, sizeof(uint8_t));
    return ESP_OK;
}


void air_quality_update_task(void* arm)
{
    while(1) {
        if (chr_air_quality_ptr != NULL) {

            /*
             * Add your Code to update the air_quality variable here
             */


            hk_notify(chr_air_quality_ptr);
        }

        vTaskDelay( 3000 / portTICK_RATE_MS );
    }
}


void app_main()
{
    ESP_LOGI(LOGNAME, "SDK version:%s\n", esp_get_idf_version());
    ESP_LOGI(LOGNAME, "Starting\n");


    // setting up homekit by using fascade
    hk_setup_add_simple_air_quality_sensor("My Air Quality Sensor", "My Company", "My Description", "0000001", "0.1", identify, air_quality_read, &chr_air_quality_ptr);

    // to restet the device. commented by default
    // hk_reset();

    // starting homekit
    hk_init("Air Quality Sensor", HK_CAT_SENSOR, "111-22-222");



    // create a task to update the temperature in the background
    xTaskCreate(
                air_quality_update_task,    /* Task function. */
                "AQTask",                 /* String with name of task. */
                10000,                      /* Stack size in bytes. */
                NULL,                       /* Parameter passed as input of the task */
                0,                          /* Priority of the task (0 = lowest) */
                NULL);                      /* Task handle. */
}
