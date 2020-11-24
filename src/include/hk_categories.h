/**
 * @file hk_categories.h
 *
 * Holdes the catagories of homekit.
 */

#pragma once

typedef enum {
    HK_CAT_OTHER = 1,
    HK_CAT_BRIDGE = 2,
    HK_CAT_FAN = 3,
    HK_CAT_GARAGE = 4,
    HK_CAT_LIGHTBULB = 5,
    HK_CAT_DOOR_LOCK = 6,
    HK_CAT_OUTLET = 7,
    HK_CAT_SWITCH = 8,
    HK_CAT_THERMOSTAT = 9,
    HK_CAT_SENSOR = 10,
    HK_CAT_SECURITY_SYSTEM = 11,
    HK_CAT_DOOR = 12,
    HK_CAT_WINDOW = 13,
    HK_CAT_WINDOW_COVERING = 14,
    HK_CAT_PROGRAMMABLE_SWITCH = 15,
    HK_CAT_RANGE_EXTENDER = 16,
    HK_CAT_IP_CAMERA = 17,
    HK_CAT_VIDEO_DOOR_BELL = 18,
    HK_CAT_AIR_PURIFIER = 19,
    HK_CAT_HEATER = 20,
    HK_CAT_AIR_CONDITIONER = 21,
    HK_CAT_HUMIDIFIER = 22,
    HK_CAT_DEHUMIDIFIER = 23,
    HK_CAT_SPEAKER = 26,
    HK_CAT_SPRINKLER = 28,
    HK_CAT_FAUCET = 29,
    HK_CAT_SHOWER_HEAD = 30,
} hk_categories_t;