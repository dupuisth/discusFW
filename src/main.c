#include <discusFW/dc_core.h>
#include <discusFW/indicator/dc_indicator.h>
#include <discusFW/mqtt/dc_mqtt.h>
#include <discusFW/wifi/dc_wifi.h>

void app_main(void)
{
  ESP_ERROR_CHECK(dc_core_init());

  dc_indicator_set_state(DC_INDICATOR_STATE_BOOTING);

  ESP_LOGI(TAG, "Device ID: %s", dc_device_id);

  if (dc_wifi_init_sta() != ESP_OK)
  {
    dc_indicator_set_state(DC_INDICATOR_STATE_FATAL);
    ESP_LOGE(TAG, "Wi-Fi init failed");
    return;
  }

  if (dc_mqtt_start() != ESP_OK)
  {
    dc_indicator_set_state(DC_INDICATOR_STATE_FATAL);
    ESP_LOGE(TAG, "MQTT start failed");
    return;
  }

  ESP_LOGI(TAG, "discusFW started");

  dc_indicator_set_state(DC_INDICATOR_STATE_OK);
  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}