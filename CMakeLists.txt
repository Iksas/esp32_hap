set(COMPONENT_SRCDIRS src src/common src/crypto src/stacks src/utils )
set(COMPONENT_REQUIRES nvs_flash json esp32_hap_wolfssl)

if(CONFIG_ESP32_HAP_STACK_IP)
    list(APPEND COMPONENT_SRCDIRS src/stacks/ip)
    list(APPEND COMPONENT_REQUIRES nghttp esp_http_server mdns)
endif()

if(CONFIG_ESP32_HAP_STACK_BLE)
    list(APPEND COMPONENT_SRCDIRS src/stacks/ble)
endif()

set(COMPONENT_ADD_INCLUDEDIRS src/include)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG")
register_component()