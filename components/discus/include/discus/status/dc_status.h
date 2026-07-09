#ifndef DC_STATUS_H
#define DC_STATUS_H

#include <esp_err.h>
#include <discus/status/dc_status_types.h>

/// @brief Update the status of a given domain
/// @param domain domain to update
/// @param level new level
/// @param detail_code optional detail code
esp_err_t dc_status_manager_set(dc_status_domain_t domain, dc_status_level_t level, int detail_code);

/// @brief Returns the current status of a given domain
/// @param domain domain
/// @param entry result
esp_err_t dc_status_manager_get(dc_status_domain_t domain, dc_status_entry_t* const entry);

/// @brief Register a callback when a status change
/// @param sync_status if the status should also be sent (resync with current state)
esp_err_t dc_status_manager_register_callback(dc_status_manager_set_cb_t callback, int sync_status);

/// @brief Broadcasts the current states (useful if a module is initialized later and need the status)
esp_err_t dc_status_manager_broadcast_status();

/// @brief Send the current states to a specific callback function (useful if a module is initialized later and the status)
esp_err_t dc_status_manager_send_status(dc_status_manager_set_cb_t callback);

#endif