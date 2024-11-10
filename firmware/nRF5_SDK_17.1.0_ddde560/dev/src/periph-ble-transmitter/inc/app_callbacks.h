/**
 * function calls for event triggers
 */

#pragma once

#include "app_common.h"
#include "app_debug.h"
#include "app_scheduler.h"

// callback function declaration macro
#define WEAK_CALLBACK_DEF(name) \
        __WEAK void callback_##name(void) { return; }
#define CALLBACK_DEF(name) \
        void callback_##name(void)
#define CALLBACK_DEF_APP_SCHED(name) \
        static void app_sched_callback_##name(void *_p_event_data, uint16_t _event_size); \
        void callback_##name(void) { app_sched_event_put(NULL, 0, app_sched_callback_##name); } \
        static void app_sched_callback_##name(void *_p_event_data, uint16_t _event_size)
#define CALLBACK_FUNC(name) \
        callback_##name

typedef void (* callback_t)(void);

