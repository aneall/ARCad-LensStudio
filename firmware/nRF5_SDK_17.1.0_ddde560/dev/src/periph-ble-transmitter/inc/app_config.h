
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#pragma message "Linked App Config"


#ifndef APP_CONFIG_LOG_ENABLED
#define APP_CONFIG_LOG_ENABLED 1
#endif

#ifndef VSCODE_EDITING
#define NRF_LOG_ENABLED APP_CONFIG_LOG_ENABLED
#else   // in vscode
#define NRF_LOG_ENABLED 0
#endif

#if NRF_LOG_ENABLED
#pragma message "NRF LOG ENABLED"
#endif

#endif	// APP_CONFIG_H



