# Firmware for reading digital caliper data and transmitting over Bluetooth Low Energy (BLE)
 
This directory should be located at path `${NRF_SDK_PATH}/*/*/periph-ble-transmitter`

Adapted from the `ble_app_uart` and `ble_app_bms` Nordic SDK examples for pairing + bonding + serial BLE service. Run on an `nRF52832` SoC with an external HF oscillator, internal RC oscillator, and DC/DC for power. 

## Apps (developed during *Immerse The Bay 2024*)

### 

Deserializer (`app_deserializer.c/h`)

* Handles low level data processing of digital caliper bitstream. 
* Publishes collected data over BLE

Alive timer (`app_alive_timer.c/h`)

* Test app for periodically printing debug information over RTT. 
* Contains a 1s heartbeat

Debug (`app_debug.c/h`)

* Not actually an application -- just a wrapper for `NRF_LOG_...()`