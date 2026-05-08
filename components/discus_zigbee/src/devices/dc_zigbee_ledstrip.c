#include <discus_zigbee/devices/dc_zigbee_ledstrip.h>
#include <esp_log.h>
#include <ha/esp_zigbee_ha_standard.h>
#include <stdlib.h>
#include <string.h>

static const char* TAG = "dc_zb_ledstrip";

esp_err_t dc_zigbee_ledstrip_create(uint8_t endpoint_id, dc_zigbee_endpoint_t* out_endpoint)
{
  return dc_zigbee_ledstrip_create_with_actions(endpoint_id, out_endpoint, NULL);
}

esp_err_t dc_zigbee_ledstrip_create_with_actions(uint8_t endpoint_id, dc_zigbee_endpoint_t* out_endpoint, dc_zigbee_ledstrip_actions_t* actions)
{
}