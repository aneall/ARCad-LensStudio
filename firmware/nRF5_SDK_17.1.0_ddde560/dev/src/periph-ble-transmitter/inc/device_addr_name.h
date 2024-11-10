/**
 * lookup specifying device address and name
 */

#pragma once

#include "ble_gap.h"

typedef struct {
    ble_gap_addr_t addr;
    char name[16];
} device_addr_t;

const device_addr_t device_name_lookup[2] = {   // addresses are little endian
    {.addr = {.addr = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}},
     .name = "ALICE"}   // TODO: set name + addr according to actual device
};
