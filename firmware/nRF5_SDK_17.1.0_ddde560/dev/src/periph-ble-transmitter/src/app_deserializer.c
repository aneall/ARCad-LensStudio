#include "app_deserializer.h"
#include "app_callbacks.h"
#include "nrfx_gpiote.h"
#include "nrf_gpio.h"
#include "app_debug.h"
#include "nrfx_timer.h"
#include "nrf_timer.h"

#define MESSAGE_TIMEOUT_US 5000


// data structures
static uint32_t bit_buffer;
static uint8_t bit_buffer_width = 0;
uint32_t dbg_clk_count = 0;

// public variables
int32_t g_measure_value = 0;
periph_units_t g_units = PERIPH_UNITS_UNINIT;

// hw config
nrfx_gpiote_in_config_t clk_pin_config = NRFX_GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
nrfx_timer_t timer = NRFX_TIMER_INSTANCE(1);
WEAK_CALLBACK_DEF(DESERIALIZER_DATA_READY)


static void parse_bit_vector_24(const uint32_t bit_vector, int32_t *value, periph_units_t *units) {
    /**
     * bit format
     * 0, [16:1] value, 20 sign, [21:22] XX, 23 units
     */
    int32_t sign = ((bit_vector >> 20) & 0x1) ? -1 : 1;
    int32_t raw_val = (bit_vector >> 1) & 0xFFFF;
    int32_t raw_units = (bit_vector >> 23) & 0x1;

    *value = sign * raw_val;
    *units = (raw_units == 0) ? PERIPH_UNITS_MM : PERIPH_UNITS_IN;
}


static void timer_timeout_handler(nrf_timer_event_t event_type, void *p_context) {
    if (event_type != NRF_TIMER_EVENT_COMPARE0) {
        return;
    }

    if (bit_buffer_width < 24) {
        debug_log("bit buffer underrun (%d), resetting", bit_buffer_width);
        bit_buffer_width = 0;
        bit_buffer = 0;
        return;
    }

    parse_bit_vector_24(bit_buffer, &g_measure_value, &g_units);
    CALLBACK_FUNC(DESERIALIZER_DATA_READY)();

    bit_buffer_width = 0;
    bit_buffer = 0;
}


static void clk_rising_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    if (pin != PERIPH_CLK_PIN) {
        return;
    }
    dbg_clk_count++;
    if (bit_buffer_width >= 24) {
        debug_log("bit buffer overrun without timeout (%d), resetting", bit_buffer_width);
        bit_buffer_width = 0;
        bit_buffer = 0;
    }

    uint8_t data = (nrf_gpio_pin_read(PERIPH_DATA_PIN) > 0) ? 1 : 0;
    bit_buffer |= (data << bit_buffer_width);
    bit_buffer_width++;

    // pet timer
    nrfx_timer_clear(&timer);
}

void timer_init() {
    nrfx_timer_config_t timer_config = NRFX_TIMER_DEFAULT_CONFIG;
    timer_config.frequency = NRF_TIMER_FREQ_1MHz;
    timer_config.bit_width = NRF_TIMER_BIT_WIDTH_32;
    nrfx_timer_init(&timer, &timer_config, timer_timeout_handler);
    nrfx_timer_enable(&timer);

    uint32_t timeout_ticks = nrfx_timer_us_to_ticks(&timer, MESSAGE_TIMEOUT_US);
    nrfx_timer_extended_compare(
        &timer, 
        NRF_TIMER_CC_CHANNEL0, 
        timeout_ticks, 
        NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, 
        true
    );
}


void deserializer_init() {
    nrfx_gpiote_in_init(PERIPH_CLK_PIN, &clk_pin_config, clk_rising_handler);
    nrfx_gpiote_in_event_enable(PERIPH_CLK_PIN, true);

    nrf_gpio_cfg_input(PERIPH_DATA_PIN, NRF_GPIO_PIN_NOPULL);

    bit_buffer = 0;
    bit_buffer_width = 0;
}


