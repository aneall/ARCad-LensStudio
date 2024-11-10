
#include "app_ble_nus.h"
#include "app_callbacks.h"
#include "device_addr_name.h"

#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "ble_hci.h"
#include "ble_nus.h"
#include "ble_gap.h"
#include "ble_conn_state.h"
#include "ble_dis.h"

#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "fds.h"

#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_ble_bms.h"

#include "nrf.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "nrf_sdh_soc.h"
#include "nrf_pwr_mgmt.h"

#include "app_timer.h"
#include "app_util_platform.h"

#define APP_BLE_CONN_CFG_TAG 1 /**< A tag identifying the SoftDevice BLE configuration. */

#define NUS_SERVICE_UUID_TYPE BLE_UUID_TYPE_VENDOR_BEGIN /**< UUID type for the Nordic UART Service (vendor specific). */

#define APP_BLE_OBSERVER_PRIO 3 /**< Application's BLE observer priority. You shouldn't need to modify this value. */

BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT); /**< BLE NUS service instance. */
NRF_BLE_BMS_DEF(m_bms);                           //!< Structure used to identify the Bond Management service.
NRF_BLE_GATT_DEF(m_gatt);                         /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                           /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);               /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;               /**< Handle of the current connection. */
static uint8_t m_qwr_mem[QWR_BUFFER_SIZE];                             //!< Write buffer for the Queued Write module.
static uint16_t m_ble_nus_max_data_len = BLE_GATT_ATT_MTU_DEFAULT - 3; /**< Maximum length of data (in bytes) that can be transmitted to the peer by the Nordic UART service module. */
static ble_conn_state_user_flag_id_t m_bms_bonds_to_delete;            //!< Flags used to identify bonds that should be deleted.
static ble_uuid_t m_adv_uuids[] =                                      /**< Universally unique service identifier. */
    {{BLE_UUID_NUS_SERVICE, NUS_SERVICE_UUID_TYPE}};

static volatile bool ble_connected = false;
static volatile bool ble_notifications_en = false;
static volatile bool ble_advertising = false;


/**@brief Clear bond information from persistent storage.
 */
static void delete_bonds(void) {
    ret_code_t err_code;

    debug_log("erase bonds");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}

/**@brief Function for handling events from bond management service.
 */
void bms_evt_handler(nrf_ble_bms_t * p_ess, nrf_ble_bms_evt_t * p_evt) {
    ret_code_t err_code;
    bool is_authorized = true;

    switch (p_evt->evt_type) {
    case NRF_BLE_BMS_EVT_AUTH:
        debug_log("authorization request");
        err_code = nrf_ble_bms_auth_response(&m_bms, is_authorized);
        APP_ERROR_CHECK(err_code);
    }
}

uint16_t qwr_evt_handler(nrf_ble_qwr_t * p_qwr, nrf_ble_qwr_evt_t * p_evt) {
    return nrf_ble_bms_on_qwr_evt(&m_bms, p_qwr, p_evt);
}

/**@brief Function for deleting a single bond if it does not belong to a connected peer.
 *
 * This will mark the bond for deferred deletion if the peer is connected.
 */
static void bond_delete(uint16_t conn_handle, void * p_context) {
    UNUSED_PARAMETER(p_context);
    ret_code_t   err_code;
    pm_peer_id_t peer_id;

    if (ble_conn_state_status(conn_handle) == BLE_CONN_STATUS_CONNECTED) {
        ble_conn_state_user_flag_set(conn_handle, m_bms_bonds_to_delete, true);
    }
    else {
        debug_log("attempting to delete bond");
        err_code = pm_peer_id_get(conn_handle, &peer_id);
        APP_ERROR_CHECK(err_code);
        if (peer_id != PM_PEER_ID_INVALID) {
            err_code = pm_peer_delete(peer_id);
            APP_ERROR_CHECK(err_code);
            ble_conn_state_user_flag_set(conn_handle, m_bms_bonds_to_delete, false);
        }
    }
}

/**@brief Function for performing deferred deletions.
*/
static void delete_disconnected_bonds(void) {
    uint32_t n_calls = ble_conn_state_for_each_set_user_flag(m_bms_bonds_to_delete, bond_delete, NULL);
    UNUSED_RETURN_VALUE(n_calls);
}

/**@brief Function for marking the requester's bond for deletion.
*/
static void delete_requesting_bond(nrf_ble_bms_t const * p_bms) {
    debug_log("Client requested that bond to current device deleted");
    ble_conn_state_user_flag_set(p_bms->conn_handle, m_bms_bonds_to_delete, true);
}


/**@brief Function for deleting all bonds
*/
static void delete_all_bonds(nrf_ble_bms_t const * p_bms) {
    ret_code_t err_code;
    uint16_t conn_handle;

    debug_log("Client requested that all bonds be deleted");

    pm_peer_id_t peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    while (peer_id != PM_PEER_ID_INVALID) {
        err_code = pm_conn_handle_get(peer_id, &conn_handle);
        APP_ERROR_CHECK(err_code);

        bond_delete(conn_handle, NULL);

        peer_id = pm_next_peer_id_get(peer_id);
    }
}


/**@brief Function for deleting all bet requesting device bonds
*/
static void delete_all_except_requesting_bond(nrf_ble_bms_t const * p_bms) {
    ret_code_t err_code;
    uint16_t conn_handle;

    debug_log("Client requested that all bonds except current bond be deleted");

    pm_peer_id_t peer_id = pm_next_peer_id_get(PM_PEER_ID_INVALID);
    while (peer_id != PM_PEER_ID_INVALID) {
        err_code = pm_conn_handle_get(peer_id, &conn_handle);
        APP_ERROR_CHECK(err_code);

        /* Do nothing if this is our own bond. */
        if (conn_handle != p_bms->conn_handle) {
            bond_delete(conn_handle, NULL);
        }

        peer_id = pm_next_peer_id_get(peer_id);
    }
}

/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for handling Service errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void service_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

const char default_name[] = DEVICE_NAME_DEFAULT;
static const char * assign_device_name(void) {
    ble_gap_addr_t dev_addr;
    sd_ble_gap_addr_get(&dev_addr);
    uint8_t num_assigned = sizeof(device_name_lookup) / sizeof(*device_name_lookup);

    debug_log("device mac: %02X:%02X:%02X:%02X:%02X:%02X",
                  dev_addr.addr[0], dev_addr.addr[1], dev_addr.addr[2],
                  dev_addr.addr[3], dev_addr.addr[4], dev_addr.addr[5]);

    for (uint8_t i = 0; i < sizeof(device_name_lookup) / sizeof(*device_name_lookup); i++) {
        if (memcmp(device_name_lookup[i].addr.addr, dev_addr.addr, BLE_GAP_ADDR_LEN) == 0) {
            debug_log("device name assigned: %s", device_name_lookup[i].name);
            return device_name_lookup[i].name;
        }
    }
    debug_log("device address not recognized. assigning default name.");
    return default_name;
}

/**@brief Function for the GAP initialization.
 *
 * @details This function will set up all the necessary GAP (Generic Access Profile) parameters of
 *          the device. It also sets the permissions and appearance.
 */
static int gap_params_init(void) {
    uint32_t err_code;
    ble_gap_conn_params_t gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    const char * dev_name = assign_device_name();
    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)dev_name,
                                          strlen(dev_name));
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
    if (err_code != NRF_SUCCESS) return err_code;

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    return (err_code != NRF_SUCCESS) ? 1 : 0;
}


WEAK_CALLBACK_DEF(BLE_NUS_EVT_RX_DATA)
WEAK_CALLBACK_DEF(BLE_NUS_EVT_TX_RDY)
WEAK_CALLBACK_DEF(BLE_NUS_EVT_COMM_STARTED)
WEAK_CALLBACK_DEF(BLE_NUS_EVT_COMM_STOPPED)

/**@brief Function for handling the data from the Nordic UART Service.
 *
 * @details This function will process the data received from the Nordic UART BLE Service and send
 *          it to the UART module.
 *
 * @param[in] p_evt       Nordic UART Service event.
 */
/**@snippet [Handling the data received over BLE] */
static void nus_data_handler(ble_nus_evt_t *p_evt) {
    switch (p_evt->type) {
    case BLE_NUS_EVT_RX_DATA:
        CALLBACK_FUNC(BLE_NUS_EVT_RX_DATA)();
        break;
    case BLE_NUS_EVT_TX_RDY:
        CALLBACK_FUNC(BLE_NUS_EVT_TX_RDY)();
        break;
    case BLE_NUS_EVT_COMM_STARTED:
        ble_notifications_en = true;
        CALLBACK_FUNC(BLE_NUS_EVT_COMM_STARTED)();
        break;
    case BLE_NUS_EVT_COMM_STOPPED:
        ble_notifications_en = false;
        CALLBACK_FUNC(BLE_NUS_EVT_COMM_STOPPED)();
        break;
    default: // Should not reach here
        break;
    }
}

/**@snippet [Handling the data received over BLE] */

/**@brief Function for initializing services that will be used by the application.
 */
static int services_init(void) {
    uint32_t            err_code;
    ble_nus_init_t      nus_init;
    ble_dis_init_t      dis_init;
    nrf_ble_bms_init_t  bms_init;
    nrf_ble_qwr_init_t  qwr_init;

    // Initialize Queued Write Module.
    memset(&qwr_init, 0, sizeof(qwr_init));
    qwr_init.mem_buffer.len   = QWR_BUFFER_SIZE;
    qwr_init.mem_buffer.p_mem = m_qwr_mem;
    qwr_init.callback         = qwr_evt_handler;
    qwr_init.error_handler    = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    debug_log("qwr init: %d", err_code);
    if (err_code != NRF_SUCCESS) return err_code;

    // Initialize NUS.
    memset(&nus_init, 0, sizeof(nus_init));

    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    debug_log("nus init: %d", err_code);
    if (err_code != NRF_SUCCESS) return err_code;

    // Initialize Bond Management Service
    memset(&bms_init, 0, sizeof(bms_init));

    m_bms_bonds_to_delete        = ble_conn_state_user_flag_acquire();
    bms_init.evt_handler         = bms_evt_handler;
    bms_init.error_handler       = service_error_handler;
    bms_init.feature.delete_requesting              = true;
    bms_init.feature.delete_all                     = true;
    bms_init.feature.delete_all_but_requesting      = true;
    bms_init.bms_feature_sec_req = SEC_JUST_WORKS;
    bms_init.bms_ctrlpt_sec_req  = SEC_JUST_WORKS;

    bms_init.p_qwr                                       = &m_qwr;
    bms_init.bond_callbacks.delete_requesting            = delete_requesting_bond;
    bms_init.bond_callbacks.delete_all                   = delete_all_bonds;
    bms_init.bond_callbacks.delete_all_except_requesting = delete_all_except_requesting_bond;

    err_code = nrf_ble_bms_init(&m_bms, &bms_init);
    debug_log("bms init: %d", err_code);
    if (err_code != NRF_SUCCESS) return err_code;

    return (err_code != NRF_SUCCESS) ? 1 : 0;
}

WEAK_CALLBACK_DEF(BLE_CONN_PARAMS_EVT_SUCCEEDED);
WEAK_CALLBACK_DEF(BLE_CONN_PARAMS_EVT_FAILED);

/**@brief Function for handling an event from the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module
 *          which are passed to the application.
 *
 * @note All this function does is to disconnect. This could have been done by simply setting
 *       the disconnect_on_fail config parameter, but instead we use the event handler
 *       mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t *p_evt) {
    uint32_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_SUCCEEDED) {
        CALLBACK_FUNC(BLE_CONN_PARAMS_EVT_SUCCEEDED)();
    }
    else if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED) {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
        CALLBACK_FUNC(BLE_CONN_PARAMS_EVT_FAILED)();
    }
}

/**@brief Function for handling errors from the Connection Parameters module.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error) {
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing the Connection Parameters module.
 */
static int conn_params_init(void) {
    uint32_t err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail = false;
    cp_init.evt_handler = on_conn_params_evt;
    cp_init.error_handler = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    return (err_code != NRF_SUCCESS) ? 1 : 0;
}

/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt) {
    uint32_t err_code;

    switch (ble_adv_evt) {
    case BLE_ADV_EVT_FAST:
        ble_advertising = true;
        break;
    case BLE_ADV_EVT_IDLE:  // advertising stopped -- restart
        ble_advertising = false;
        nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_RESET);
        break;
    default:
        break;
    }
}

WEAK_CALLBACK_DEF(BLE_GAP_EVT_CONNECTED)
WEAK_CALLBACK_DEF(BLE_GAP_EVT_DISCONNECTED)
WEAK_CALLBACK_DEF(BLE_GAP_EVT_PHY_UPDATE_REQUEST)
WEAK_CALLBACK_DEF(BLE_GATTC_EVT_TIMEOUT)
WEAK_CALLBACK_DEF(BLE_GATTS_EVT_TIMEOUT)

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const *p_ble_evt, void *p_context) {
    uint32_t err_code;

    pm_handler_secure_on_connection(p_ble_evt);

    switch (p_ble_evt->header.evt_id) {
    case BLE_GAP_EVT_CONNECTED:
        ble_connected = true;
        m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
        APP_ERROR_CHECK(err_code);
        CALLBACK_FUNC(BLE_GAP_EVT_CONNECTED)();
        break;

    case BLE_GAP_EVT_DISCONNECTED:
        ble_connected = false;
        delete_disconnected_bonds();
        m_conn_handle = BLE_CONN_HANDLE_INVALID;
        CALLBACK_FUNC(BLE_GAP_EVT_DISCONNECTED)();
        break;

    case BLE_GAP_EVT_PHY_UPDATE_REQUEST: {
        ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
        err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
        APP_ERROR_CHECK(err_code);
        CALLBACK_FUNC(BLE_GAP_EVT_PHY_UPDATE_REQUEST)();
    } break;

    case BLE_GATTC_EVT_TIMEOUT:
        // Disconnect on GATT Client timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        CALLBACK_FUNC(BLE_GATTC_EVT_TIMEOUT)();
        break;

    case BLE_GATTS_EVT_TIMEOUT:
        // Disconnect on GATT Server timeout event.
        err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                         BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
        APP_ERROR_CHECK(err_code);
        CALLBACK_FUNC(BLE_GATTS_EVT_TIMEOUT)();
        break;

    default:
        // No implementation needed.
        break;
    }
}

/**@brief Function for the SoftDevice initialization.
 *
 * @details This function initializes the SoftDevice and the BLE event interrupt.
 */
static int ble_stack_init(void) {
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    if (err_code != NRF_SUCCESS) return err_code;

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    if (err_code != NRF_SUCCESS) return err_code;

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    if (err_code != NRF_SUCCESS) return err_code;

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
    return 0;
}

WEAK_CALLBACK_DEF(NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)

/**@brief Function for handling events from the GATT library. */
void gatt_evt_handler(nrf_ble_gatt_t *p_gatt, nrf_ble_gatt_evt_t const *p_evt) {
    if ((m_conn_handle == p_evt->conn_handle) && (p_evt->evt_id == NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)) {
        m_ble_nus_max_data_len = p_evt->params.att_mtu_effective - OPCODE_LENGTH - HANDLE_LENGTH;
        CALLBACK_FUNC(NRF_BLE_GATT_EVT_ATT_MTU_UPDATED)();
    }
}

/**@brief Function for initializing the GATT library. */
static int gatt_init(void) {
    ret_code_t err_code;

    err_code = nrf_ble_gatt_init(&m_gatt, gatt_evt_handler);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = nrf_ble_gatt_att_mtu_periph_set(&m_gatt, NRF_SDH_BLE_GATT_MAX_MTU_SIZE);
    return (err_code != NRF_SUCCESS) ? 1 : 0;
}

/**@brief Function for handling Peer Manager events.
 *
 * @param[in] p_evt  Peer Manager event.
 */
static void pm_evt_handler(pm_evt_t const * p_evt) {
    pm_handler_on_pm_evt(p_evt);
    // pm_handler_disconnect_on_sec_failure(p_evt);
    if (p_evt->evt_id == PM_EVT_CONN_SEC_FAILED) {
        debug_log("security failed, ignoring");
    }
    pm_handler_flash_clean(p_evt);

    switch (p_evt->evt_id) {
    case PM_EVT_PEERS_DELETE_SUCCEEDED:
        advertising_start(false);
        break;

    default:
        break;
    }
}


/**@brief Function for the Peer Manager initialization.
 */
static int peer_manager_init(void) {
    ble_gap_sec_params_t sec_param;
    ret_code_t err_code;

    err_code = pm_init();
    if (err_code != NRF_SUCCESS) return err_code;

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    if (err_code != NRF_SUCCESS) return err_code;

    err_code = pm_register(pm_evt_handler);
    return (err_code != NRF_SUCCESS) ? 1 : 0;
}

/**@brief Function for initializing the Advertising functionality.
 */
static int advertising_init(void) {
    uint32_t err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type = ENABLE_DEVICE_NAME ? BLE_ADVDATA_FULL_NAME : BLE_ADVDATA_NO_NAME;
    init.advdata.include_appearance = false;
    init.advdata.flags = BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout = APP_ADV_DURATION;
    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    if (err_code != NRF_SUCCESS) return err_code;

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
    return 0;
}

/**
 * public functions -----------------------------------------------------------
 */

/**
 * @brief initialize BLE stack
 */
void ble_all_services_init(void) {
    ret_code_t err_code;

    err_code = ble_stack_init();
    if (err_code) {
        debug_log("ble_stack_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    err_code = gap_params_init();
    if (err_code) {
        debug_log("gap_params_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    err_code = gatt_init();
    if (err_code) {
        debug_log("gatt_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    err_code = services_init();
    if (err_code) {
        debug_log("services_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    err_code = advertising_init();
    if (err_code) {
        debug_log("advertising_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    err_code = conn_params_init();
    if (err_code) {
        debug_log("conn_params_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    err_code = peer_manager_init();
    if (err_code) {
        debug_log("peer_manager_init() failed with error code %d", err_code);
        goto BLE_ALL_SERVICES_INIT_FAIL;
    }

    BLE_ALL_SERVICES_INIT_FAIL:
    if (err_code) {
        debug_flush();
        APP_ERROR_CHECK(NRF_ERROR_INTERNAL);
    }
}

/**
 * @brief start advertising. ble stack should be initialized first
 */
void advertising_start(bool erase_bonds) {
    if (erase_bonds) {  // Advertising is started by PM_EVT_PEERS_DELETE_SUCCEEDED event.
        delete_bonds();
        return;
    }

    if (ble_advertising) return; // already advertising
    debug_log("advertising start"); debug_force_flush();
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief stop advertising.
 */
void advertising_stop(void) {
    if (!ble_advertising) return; // not advertising
    debug_log("advertising stop");
    uint32_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_IDLE);
    APP_ERROR_CHECK(err_code);
    sd_ble_gap_adv_stop(m_advertising.adv_handle);
    APP_ERROR_CHECK(err_code);
}

/**
 * @brief send data over BLE NUS
 * @return NRF_SUCCESS if successful, otherwise an error code
 *         NRF_ERROR_INVALID_STATE if not connected or notifications not enabled
 */
ret_code_t ble_send(uint8_t *data, uint16_t length) {
    if (!ble_connected || !ble_notifications_en) {
        return NRF_ERROR_INVALID_STATE;
    }

    return ble_nus_data_send(&m_nus, data, &length, m_conn_handle);
}

/**
 * @brief force a ble disconnection
 */
void ble_disconnect(bool stop_advertising) {
    sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
    if (stop_advertising) advertising_stop();
}
