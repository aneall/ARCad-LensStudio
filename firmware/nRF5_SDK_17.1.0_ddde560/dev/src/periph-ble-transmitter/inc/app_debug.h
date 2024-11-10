/**
 * wrapper for NRF LOG and other debug functionality
*/

#pragma once

#include "app_common.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"

void debug_init(void);
static inline bool debug_process(void) { return NRF_LOG_PROCESS(); }
static inline void debug_flush(void)   { NRF_LOG_FLUSH(); }
static inline void debug_force_flush(void) {
    NRF_LOG_FLUSH();
    NRF_LOG_PROCESS();
}
#define debug_log(...)  \
        do { \
            NRF_LOG_INFO(__VA_ARGS__); \
        } while (0)
