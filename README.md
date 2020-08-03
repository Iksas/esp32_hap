# esp32-homekit
ESP-32 implementation of Apple Homekit Accessory Protocol(HAP)

## Usage
To use the component on macos/linux, do the following:
1. Setup Espressif development environment. This project is tested with ESP-IDF version 4.1: https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html#
1. Create your application
1. Install submodules
    git submodule add https://github.com/slompf18/esp32_hap_wolfssl.git components/esp32_hap_wolfssl
    git submodule add https://github.com/slompf18/esp32-homekit.git components/esp32-homekit

## Unit testing
1. cd into the test_runner directory
2. idf.py menuconfig
3. choose the stack of hap
4. enable bluetooth (nimble only)
5. disable task watchdog
6. build and flash app

## Debugging
### Set log level
In order to get more (or less) verbosity, change the following line in CMakeLists.txt: set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLOG_LOCAL_LEVEL=ESP_LOG_DEBUG")

### How to WireShark the IOs Device
1. Connect device by USB
2. Get your UDID: http://whatsmyudid.com/
3. Check your network devices: ifconfig -l
4. Start capturing: rvictl -s <UDID>
5. Check your network devices again: ifconfig -l
6. Open WireShark
7. Start capturing on the new interface (normally rvi0)
8. Filter for http

### Debug mDNS/Bonjour/Zeroconf
HomeKit uses the service type of '_hap._tcp' on the 'local.' domain, so you can query for HomeKit devices as such:

$ dns-sd -Z _hap._tcp local.

For HomeKit hubs, look for the service type of _homekit._tcp:

$ dns-sd -Z _homekit._tcp local.

## Troubleshooting
- To be able to upload to the EP-WROOM-32 when connected to a breadboard, you may need a 10K (or so) pull-down resistor between RX0 and GND.
- In case you cannot find your device, it may be a problem with your router: https://jlian.co/post/reliable-homekit/