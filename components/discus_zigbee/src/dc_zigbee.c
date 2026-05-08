#include <discus/core/dc_core.h>
#include <discus_zigbee/dc_zigbee.h>
#include <esp_check.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ha/esp_zigbee_ha_standard.h>

static const char* TAG = "dc_zigbee";

static dc_zigbee_device_t* s_device;

static const dc_zigbee_endpoint_t* dc_zigbee_find_endpoint(uint8_t endpoint_id)
{
  if (!s_device)
  {
    return NULL;
  }

  for (size_t i = 0; i < s_device->endpoint_count; i++)
  {
    if (s_device->endpoints[i].endpoint_id == endpoint_id)
    {
      return &s_device->endpoints[i];
    }
  }

  return NULL;
}

static esp_err_t dc_zigbee_register_device(const dc_zigbee_device_t* device)
{
  ESP_RETURN_ON_FALSE(device, ESP_ERR_INVALID_ARG, TAG, "Missing Zigbee device");
  ESP_RETURN_ON_FALSE(device->endpoints, ESP_ERR_INVALID_ARG, TAG, "Missing endpoints");
  ESP_RETURN_ON_FALSE(device->endpoint_count > 0, ESP_ERR_INVALID_ARG, TAG, "Zigbee device must expose at least 1 endpoint");

  // Zigbee list of endpoints
  esp_zb_ep_list_t* ep_list = esp_zb_ep_list_create();
  ESP_RETURN_ON_FALSE(ep_list, ESP_ERR_NO_MEM, TAG, "Failed to allocate Zigbee endpoint list");

  // Convert the DC endpoints to ESP endoints
  for (size_t i = 0; i < device->endpoint_count; i++)
  {
    const dc_zigbee_endpoint_t* endpoint = &device->endpoints[i];
    esp_zb_cluster_list_t* clusters = endpoint->clusters;
    if (!clusters && endpoint->create_clusters)
    {
      clusters = endpoint->create_clusters(endpoint->ctx);
    }
    ESP_RETURN_ON_FALSE(endpoint->clusters, ESP_ERR_INVALID_ARG, TAG, "Endpoint %u has no clusters", endpoint->endpoint_id);

    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = endpoint->endpoint_id,
        .app_profile_id = endpoint->profile_id,
        .app_device_id = endpoint->device_id,
        .app_device_version = 0,
    };

    ESP_RETURN_ON_ERROR(
        esp_zb_ep_list_add_ep(ep_list, endpoint->clusters, endpoint_config), TAG, "Failed to add endpoints %u", endpoint->endpoint_id);
  }

  // The list is built, now sent it to the stack (the zb datamodels are automatically freed and should not be reused)
  return esp_zb_device_register(ep_list);
}

static esp_err_t dc_zigbee_action_handler(esp_zb_core_action_callback_id_t callback_id, const void* message)
{
  if (callback_id != ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID)
  {
    return ESP_OK;
  }

  const esp_zb_zcl_set_attr_value_message_t* msg = (const esp_zb_zcl_set_attr_value_message_t*)message;
  if (!msg || msg->info.status != ESP_ZB_ZCL_STATUS_SUCCESS)
  {
    return ESP_ERR_INVALID_ARG;
  }

  const dc_zigbee_endpoint_t* endpoint = dc_zigbee_find_endpoint(msg->info.dst_endpoint);
  if (!endpoint || !endpoint->on_attr_write)
  {
    return ESP_OK;
  }

  return endpoint->on_attr_write(msg->info.dst_endpoint, msg->info.cluster, msg->attribute.id, msg->attribute.data.value, endpoint->ctx);
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t* signal_s)
{
  uint32_t* app_signal = signal_s->p_app_signal;
  esp_err_t err_status = signal_s->esp_err_status;
  esp_zb_app_signal_type_t signal_type = *app_signal;

  ESP_LOGI(TAG, "Zigbee signal: %s status: %s", esp_zb_zdo_signal_to_string(signal_type), esp_err_to_name(err_status));

  esp_err_t err;
  switch (signal_type)
  {
  case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
    err = esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
    if (err)
    {
      dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_FATAL);
      ESP_ERROR_CHECK(err);
    }
    break;
  case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
  case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
    if (err_status == ESP_OK)
    {
      err = esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
      if (err)
      {
        dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_FATAL);
        ESP_ERROR_CHECK(err);
      }
    }
    else
    {
      dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_ERROR);
    }
    break;
  case ESP_ZB_BDB_SIGNAL_STEERING:
    dc_indicator_set_state(&dc_indicator_connectivity, err_status == ESP_OK ? DC_INDICATOR_STATE_SUCCESS : DC_INDICATOR_STATE_WARNING);
    break;
  }
}

static void dc_zb_task(void* pvParameters)
{
  esp_err_t err = ESP_OK;
  const dc_zigbee_device_t* device = (const dc_zigbee_device_t*)pvParameters;
  esp_zb_cfg_t zb_nwk_cfg = DC_ZB_ZED_CONFIG();

  esp_zb_init(&zb_nwk_cfg);

  err = dc_zigbee_register_device(device);
  if (err)
  {
    dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_ERROR);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to register Zigbee device");
  }

  esp_zb_core_action_handler_register(dc_zigbee_action_handler);
  err = esp_zb_set_primary_network_channel_set(DC_ZB_PRIMARY_CHANNEL_MASK);
  if (err)
  {
    dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_ERROR);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set primary network channel");
  }

  err = esp_zb_start(false);
  if (err)
  {
    dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_ERROR);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to start Zigbee");
  }
  esp_zb_stack_main_loop();
}

esp_err_t dc_zigbee_init(dc_zigbee_config_t* config)
{
  esp_err_t err = ESP_OK;
  dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_SETUP);

  esp_zb_platform_config_t platform_config = {.radio_config = DC_ZB_DEFAULT_RADIO_CONFIG(), .host_config = DC_ZB_DEFAULT_HOST_CONFIG()};

  err = esp_zb_platform_config(&platform_config);
  if (err)
  {
    dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_ERROR);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to configure Zigbee platform");
  }

  s_device = config->device;
  BaseType_t task_created = xTaskCreate(dc_zb_task, "Zigbee", 4096, (void*)s_device, 5, NULL);
  if (task_created != pdPASS)
  {
    dc_indicator_set_state(&dc_indicator_connectivity, DC_INDICATOR_STATE_ERROR);
    err = ESP_ERR_NO_MEM;
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to create Zigbee task");
  }
}