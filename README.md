# ESP 32 Homekit stack
ESP-32 implementation of Apple Homekit Accessory Protocol(HAP). It supports Homekit via Bluetooth and wlan.

# Getting started
1. Setup Espressif development environment. Go through steps 1 to 10 in this [guide (v4.2)](https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/get-started/index.html).
2. Make your project directory a git repository:
   ```
   %> git init
   %> echo 'build' > .gitignore
   ```
3. Install submodules in the project you just created ($IDF_PATH/examples/get-started/hello_world). To do this, isssue:

    ```bash
    %> git submodule add https://github.com/slompf18/esp32_hap_wolfssl.git components/esp32_hap_wolfssl
    %> git submodule add https://github.com/Iksas/esp32_hap.git components/esp32_hap
    ```
4. Overwrite the content of 'hello_world_main.c' with the content of 'examples\simple_switch_ble.c' of this repository.
5. Add the following line into CMakeLists.txt to add the esp32_hap component:
   ```
    set(COMPONENTS esptool_py esp32 esp32_hap main)
    ```
6. Enable Bluetooth LE by opening the configuration `idf.py menuconfig`. Then select:
    1. Component config -> Bluetooth: Enable
    2. Component config -> Bluetooth -> Bluetooth Host: NimBLE - BLE only
    3. Save and Quit.
7. Connect an led to GND and IO 19 of your ESP32.
7. Flash and monitor the project to the esp.

# Bluetooth or Wlan
When starting your project you will have to choose, which stack you want to use. You can choose your stack by calling `idf.py menuconfig` and selecting the option under the 'Homekit' menu entry. Here are the different advantages:

## Bluetooth
- Easy configuration
- Uses less ressources

## Wlan
- Higher range

# Supported BLE accessories


| Accessory  | Working Characteristics | Non-working Characteristics |
| ------------- | ------------- | ------------- |
| Switch | Name, On  | -  |
| Temperature Sensor  | Current Temperature, Name  | -  |

All accessories and characteristics not listed above have yet to be tested.

# Develop

## Unit testing
1. cd into the test_runner directory
2. idf.py menuconfig
3. choose the stack of hap
4. enable bluetooth (nimble only)
5. disable task watchdog
6. build and flash the test app: idf.py -p /dev/tty.SLAB_USBtoUART flash monitor

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
