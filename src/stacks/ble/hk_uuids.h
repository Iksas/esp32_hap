#pragma once

#include <stdbool.h>
#include "host/ble_uuid.h"

#define HK_UUIDS_ASSIGN_NAME(var_name, uuid) \
    char var_name[50];                     \
    hk_uuids_to_name(uuid, var_name)

const static ble_uuid128_t hk_uuids_srv_id = BLE_UUID128_INIT(0xd1, 0xa0, 0x83, 0x50, 0x00, 0xaa, 0xd3, 0x87,
                                                              0x17, 0x48, 0x59, 0xa7, 0x5d, 0xe9, 0x04, 0xe6);

const static ble_uuid128_t hk_uuids_descriptor_instance_id =
    BLE_UUID128_INIT(0x9a, 0x93, 0x96, 0xd7, 0xbd, 0x6a, 0xd9, 0xb5,
                     0x16, 0x46, 0xd2, 0x81, 0xfe, 0xf0, 0x46, 0xdc);

const static ble_uuid128_t hk_uuids_descriptor_client_configuration =
    BLE_UUID128_INIT(0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80,
                     0x00, 0x10, 0x00, 0x00, 0x02, 0x29, 0x00, 0x00);

ble_uuid128_t *hk_uuids_get(uint8_t id);
bool hk_uuids_cmp(const ble_uuid128_t *uuid1, const ble_uuid128_t *uuid2);
char *hk_uuids_to_str(const ble_uuid128_t *uuid);
void hk_uuids_to_name(const ble_uuid128_t *uuid, char *str);