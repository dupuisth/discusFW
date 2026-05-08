#ifndef DC_ZIGBEE_DEVICE_H
#define DC_ZIGBEE_DEVICE_H

#include <esp_err.h>
#include <esp_zigbee_core.h>
#include <stddef.h>
#include <stdint.h>

typedef esp_err_t (*dc_zigbee_attr_write_cb_t)(uint8_t endpoint_id, uint16_t cluster_id, uint16_t attr_id, const void* value, void* ctx);
typedef esp_zb_cluster_list_t* (*dc_zigbee_cluster_create_cb_t)(void* ctx);

typedef struct dc_zigbee_endpoint
{
  uint8_t endpoint_id;
  uint16_t profile_id;
  uint16_t device_id;

  esp_zb_cluster_list_t* clusters;
  dc_zigbee_cluster_create_cb_t create_clusters;

  dc_zigbee_attr_write_cb_t on_attr_write;
  void* ctx;
} dc_zigbee_endpoint_t;

typedef struct dc_zigbee_device
{
  const char* manufacturer;
  const char* model;
  const dc_zigbee_endpoint_t* endpoints;
  size_t endpoint_count;
} dc_zigbee_device_t;

#endif
