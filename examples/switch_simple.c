#define LOGNAME "SISWI"

#include <hk.h>
#include <hk_fascade.h>

#include <esp_system.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>

#define LED 19
bool value = false;

void on_identify()
{
    ESP_LOGI(LOGNAME, "Identify");
}

esp_err_t on_write(hk_mem *request)
{
    value = *((bool *)request->ptr);
    ESP_LOGI(LOGNAME, "Setting led to %d", value);
    gpio_set_level(LED, !value);

    return ESP_OK;
}

esp_err_t on_read(hk_mem *response)
{
    hk_mem_append_buffer(response, (char *)&value, sizeof(bool));
    return ESP_OK;
}

void app_main()
{
    ESP_LOGI(LOGNAME, "SDK version:%s\n", esp_get_idf_version());
    ESP_LOGI(LOGNAME, "Starting\n");

    // setting up led
    gpio_pad_select_gpio(LED);
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, !value);
    
    // setting up homekit by using fascade
    void *chr_ptr;
    hk_setup_add_switch("My Switch", "My Company", "My Description", "0000001", "0.1", on_identify, on_read, on_write, &chr_ptr);

    // to restet the device. commented by default
    // hk_reset();

    // starting homekit
    hk_init("Switch", HK_CAT_SWITCH, "111-22-222");
}