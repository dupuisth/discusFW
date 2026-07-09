#ifndef DC_STATUS_TYPES_H
#define DC_STATUS_TYPES_H

///////////////////////////////////////////
// Domains
///////////////////////////////////////////

typedef enum
{
  DC_STATUS_DOMAIN_SYSTEM = 0,
  DC_STATUS_DOMAIN_ZIGBEE,
  DC_STATUS_DOMAIN_SENSOR,
  DC_STATUS_DOMAIN_LAST_ENUM
} dc_status_domain_t;

///////////////////////////////////////////
// Status level
///////////////////////////////////////////

typedef enum
{
  DC_STATUS_LEVEL_SYSTEM_UNKNOWN,
  DC_STATUS_LEVEL_SYSTEM_DISABLED,
  DC_STATUS_LEVEL_SYSTEM_INIT,
  DC_STATUS_LEVEL_SYSTEM_OK,
  DC_STATUS_LEVEL_SYSTEM_WARNING,
  DC_STATUS_LEVEL_SYSTEM_ERROR,
  DC_STATUS_LEVEL_SYSTEM_FATAL,
} dc_status_level_t;

///////////////////////////////////////////
// Additional infos (detail_code)
///////////////////////////////////////////
typedef enum
{
  DC_STATUS_CONNECTIVITY_DISCONNECTED = 100,
  DC_STATUS_CONNECTIVITY_JOINING,
  DC_STATUS_CONNECTIVITY_CONNECTED,
  DC_STATUS_CONNECTIVITY_RECONNECTING,
  DC_STATUS_CONNECTIVITY_PAIRING,
  DC_STATUS_CONNECTIVITY_ERROR,
} dc_status_connectivity_t;

// If any other additional info types, make sur the ids won't overlap

//---------------------------------------//

typedef struct
{
  dc_status_level_t level;
  int detail_code; // Additional domain specific status (JOINING, CONNECTING, ...)
} dc_status_entry_t;

// Signature of status change callback function
typedef void (*dc_status_manager_set_cb_t)(dc_status_domain_t domain, dc_status_entry_t status);

#endif