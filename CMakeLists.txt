set(COMPONENT_SRCDIRS src src/common src/crypto src/stacks src/utils )
set(COMPONENT_PRIV_REQUIRES bt esp32_hap_wolfssl esp_http_server json mdns nghttp nvs_flash)

if(CONFIG_ESP32_HAP_STACK_IP)
    list(APPEND COMPONENT_SRCDIRS src/stacks/ip)
endif()

if(CONFIG_ESP32_HAP_STACK_BLE)
    list(APPEND COMPONENT_SRCDIRS src/stacks/ble)
endif()

set(COMPONENT_ADD_INCLUDEDIRS src/include)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG")
register_component()