#include "app_alive_timer.h"
#include "app_debug.h"
#include "app_ble_nus.h"
#include "app_deserializer.h"

APP_TIMER_DEF(m_alive_timer_id);

void alive_timer_handler(void *p_context) {
    static uint32_t heartbeat = 0;
    debug_log("alive %d", heartbeat++);
    debug_log("dbg_clk_count %d", dbg_clk_count);
}

void app_alive_timer_init(void) {
    ret_code_t err_code;
    err_code = app_timer_create(&m_alive_timer_id,
                                APP_TIMER_MODE_REPEATED,
                                alive_timer_handler);
    err_code = app_timer_start(m_alive_timer_id,
                               APP_TIMER_TICKS(1000),
                               NULL);
}
