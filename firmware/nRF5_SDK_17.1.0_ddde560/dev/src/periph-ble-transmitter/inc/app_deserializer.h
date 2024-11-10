#pragma once

#include "app_common.h"

typedef enum periph_units_e {
    PERIPH_UNITS_UNINIT = 0,
    PERIPH_UNITS_MM,
    PERIPH_UNITS_IN,
} periph_units_t;

extern int32_t g_measure_value;
extern periph_units_t g_units;
extern uint32_t dbg_clk_count;

void timer_init();
void deserializer_init();