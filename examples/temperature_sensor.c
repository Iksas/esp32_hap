#define LOGNAME "SISWI"

#include <hk.h>
#include <hk_fascade.h>

#include <esp_system.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#include "freertos/task.h"

#define LED 19
float temperature = 20.0; // values between 0 and 100 degrees celsius are supported.
void *chr_temp_ptr = NULL;

void identify()
{
    ESP_LOGI(LOGNAME, "Identify");
}


esp_err_t temperature_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&temperature, sizeof(float));
    return ESP_OK;
}


void temperature_update_task(void* arm)
{
    while(1) {
        if (chr_temp_ptr != NULL) {

            /*
             * Add your Code to update the temperature variable here
             */
            temperature += 0.1; // for testing


            hk_notify(chr_temp_ptr);
        }

        vTaskDelay( 3000 / portTICK_RATE_MS );
    }
}


void app_main()
{
    ESP_LOGI(LOGNAME, "SDK version:%s\n", esp_get_idf_version());
    ESP_LOGI(LOGNAME, "Starting\n");


    // setting up homekit by using fascade
    hk_setup_add_temperature_sensor("My Temperature Sensor", "My Company", "My Description", "0000001", "0.1", identify, temperature_read, &chr_temp_ptr);

    // to restet the device. commented by default
    // hk_reset();

    // starting homekit
    hk_init("Temperature Sensor", HK_CAT_SENSOR, "111-22-222");



    // create a task to update the temperature in the background
    xTaskCreate(
                temperature_update_task,    /* Task function. */
                "TempTask",                 /* String with name of task. */
                10000,                      /* Stack size in bytes. */
                NULL,                       /* Parameter passed as input of the task */
                0,                          /* Priority of the task (0 = lowest) */
                NULL);                      /* Task handle. */
}
