#pragma once

#include <stdlib.h>
#include <stdbool.h>

void hk_advertising_init(const char *name, size_t category, size_t config_version);
void hk_advertising_update_paired(bool initial);
