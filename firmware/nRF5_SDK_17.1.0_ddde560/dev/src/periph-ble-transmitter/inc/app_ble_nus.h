/**
 * BLE NUS functions
 */

#pragma once

#include "app_common.h"

void ble_all_services_init(void);
void advertising_start(bool erase_bonds);
void advertising_stop(void);
ret_code_t ble_send(uint8_t *data, uint16_t length);
void ble_disconnect(bool stop_advertising);
