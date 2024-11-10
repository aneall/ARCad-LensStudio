/**
 * common includes and configurations
*/

#pragma once

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include "nordic_common.h"
#include "app_util.h"
#include "app_timer.h"



// GPIO config
#define PERIPH_CLK_PIN  8
#define PERIPH_DATA_PIN 7

// test config
#define POWER_PROFILING_ENABLED 0           // enable power profiling -- runs IMU sample and BLE send on loop
#define POWER_PROFILING_PERIOD_MS 500       // period of power profiling loop

// BLE config
#define DEVICE_NAME_DEFAULT     "test"      // device name in BLE advertising
#define ENABLE_DEVICE_NAME      1           // enable device name in advertising

#define APP_ADV_INTERVAL        MSEC_TO_UNITS(64, UNIT_0_625_MS)    // advertising interval (in units of 0.625 ms)
#define APP_ADV_DURATION        MSEC_TO_UNITS(180000, UNIT_10_MS)    // advertising duration (in units of 10 milliseconds)
#define MIN_CONN_INTERVAL       MSEC_TO_UNITS(20,  UNIT_1_25_MS)     /**< Minimum acceptable connection interval (20 ms), Connection interval uses 1.25 ms units. */
#define MAX_CONN_INTERVAL       MSEC_TO_UNITS(75, UNIT_1_25_MS)     /**< Maximum acceptable connection interval (75 ms), Connection interval uses 1.25 ms units. */
#define SLAVE_LATENCY           0                                 /**< Slave latency. */
#define CONN_SUP_TIMEOUT        MSEC_TO_UNITS(4000, UNIT_10_MS)    /**< Connection supervisory timeout (4 seconds), Supervision Timeout uses 10 ms units. */
#define FIRST_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(5000) /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY APP_TIMER_TICKS(30000) /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT 1                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define QWR_BUFFER_SIZE                 512

#define SEC_PARAM_BOND                  1                                       //!< Perform bonding.
#define SEC_PARAM_MITM                  0                                       //!< Man In The Middle protection not required.
#define SEC_PARAM_LESC                  0                                       //!< LE Secure Connections not enabled.
#define SEC_PARAM_KEYPRESS              0                                       //!< Keypress notifications not enabled.
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    //!< No I/O capabilities.
#define SEC_PARAM_OOB                   0                                       //!< Out Of Band data not available.
#define SEC_PARAM_MIN_KEY_SIZE          7                                       //!< Minimum encryption key size.
#define SEC_PARAM_MAX_KEY_SIZE          16                                      //!< Maximum encryption key size.
