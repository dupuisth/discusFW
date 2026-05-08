#ifndef DC_ZIGBEE_LEDSTRIP_H
#define DC_ZIGBEE_LEDSTRIP_H

#include <discus_zigbee/dc_zigbee_device.h>
#include <esp_err.h>
#include <stdbool.h>

typedef struct dc_zigbee_ledstrip_actions
{
  esp_err_t (*set_power)(bool on, void* ctx);
  esp_err_t (*set_level)(uint8_t level, void* ctx);
  esp_err_t (*set_color_xy)(uint16_t x, uint16_t y, void* ctx);
  void* ctx
} dc_zigbee_ledstrip_actions_t;

esp_err_t dc_zigbee_ledstrip_create(uint8_t endpoint_id, dc_zigbee_endpoint_t* out_endpoint);
esp_err_t dc_zigbee_ledstrip_create_with_actions(uint8_t endpoint_id, dc_zigbee_endpoint_t* out_endpoint, dc_zigbee_ledstrip_actions_t* actions);

#endif
