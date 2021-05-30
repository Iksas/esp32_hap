#define LOGNAME "SISWI"

#include <hk.h>
#include <hk_fascade.h>

#include <esp_system.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#include "freertos/task.h"

#define LED 19
uint8_t air_quality = 2;                // values between 0 and 5 are supported.
float ozone_density = 10.5;             // values between 0 and 1000 are supported
float nitrogen_dioxide_density = 13.0;  // values between 0 and 1000 are supported
float sulphur_dioxide_density = 13.0;   // values between 0 and 1000 are supported
float pm25_density = 13.0;              // values between 0 and 1000 are supported
float pm10_density = 13.0;              // values between 0 and 1000 are supported
float voc_density = 13.0;               // values between 0 and 1000 are supported
bool status_active = true;
uint8_t status_tampered = 0;
uint8_t status_battery = 0;


void *chr_air_quality_ptr = NULL;
void *chr_ozone_density_ptr = NULL;
void *chr_nitrogen_dioxide_ptr = NULL;
void *chr_sulphir_dioxode_ptr = NULL;
void *chr_pm25_ptr = NULL;
void *chr_pm10_ptr = NULL;
void *chr_voc_ptr = NULL;
void *chr_active_ptr = NULL;
void *chr_tampered_ptr = NULL;
void *chr_battery_ptr = NULL;



void identify()
{
    ESP_LOGI(LOGNAME, "Identify");
}


esp_err_t air_quality_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&air_quality, sizeof(uint8_t));
    return ESP_OK;
}

esp_err_t ozone_density_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&ozone_density, sizeof(float));
    return ESP_OK;
}

esp_err_t nitrogen_dioxode_density_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&nitrogen_dioxide_density, sizeof(float));
    return ESP_OK;
}

esp_err_t sulphur_dioxode_density_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&sulphur_dioxide_density, sizeof(float));
    return ESP_OK;
}

esp_err_t pm25_density_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&pm25_density, sizeof(float));
    return ESP_OK;
}

esp_err_t pm10_density_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&pm10_density, sizeof(float));
    return ESP_OK;
}

esp_err_t voc_density_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&voc_density, sizeof(float));
    return ESP_OK;
}

esp_err_t active_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&status_active, sizeof(bool));
    return ESP_OK;
}

esp_err_t tampered_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&status_tampered, sizeof(uint8_t));
    return ESP_OK;
}

esp_err_t battery_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&status_battery, sizeof(uint8_t));
    return ESP_OK;
}



void data_update_task(void* arm)
{
    while(1) {
        if (chr_air_quality_ptr != NULL) {

            /*
             * Add your Code to update data here
             */
            nitrogen_dioxide_density += 0.5;



            // for some reason the WD fires if more than 2 or 3
            // characteristics are notified at a time. Resetting the WD
            // between each call didn't help.
            hk_notify(chr_air_quality_ptr);
            hk_notify(chr_nitrogen_dioxide_ptr);
    }

        vTaskDelay( 3000 / portTICK_RATE_MS );
    }
}




void app_main()
{
    ESP_LOGI(LOGNAME, "SDK version:%s\n", esp_get_idf_version());
    ESP_LOGI(LOGNAME, "Starting\n");


    // setting up homekit by using fascade
    hk_setup_add_full_air_quality_sensor("My Air Quality Sensor", "My Company", "My Description", "0000001", "0.1", identify,
        air_quality_read,
        ozone_density_read,
        nitrogen_dioxode_density_read,
        sulphur_dioxode_density_read,
        pm25_density_read,
        pm10_density_read,
        voc_density_read,
        active_read,
        tampered_read,
        battery_read,
        &chr_air_quality_ptr,
        &chr_ozone_density_ptr,
        &chr_nitrogen_dioxide_ptr,
        &chr_sulphir_dioxode_ptr,
        &chr_pm25_ptr,
        &chr_pm10_ptr,
        &chr_voc_ptr,
        &chr_active_ptr,
        &chr_tampered_ptr,
        &chr_battery_ptr
    );

    // to restet the device. commented by default
    // hk_reset();

    // starting homekit
    hk_init("Air Quality Sensor", HK_CAT_SENSOR, "111-22-222");



    // create a task to update the temperature in the background
    xTaskCreate(
                data_update_task,    /* Task function. */
                "UpdateTask",                 /* String with name of task. */
                10000,                      /* Stack size in bytes. */
                NULL,                       /* Parameter passed as input of the task */
                0,                          /* Priority of the task (0 = lowest) */
                NULL);                      /* Task handle. */
}
