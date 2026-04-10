#include <discusFW/dc_core.h>
#include <discusFW/mqtt/dc_mqtt.h>
#include <discusFW/wifi/dc_wifi.h>

void app_main(void)
{
  ESP_ERROR_CHECK(dc_core_init());

  ESP_LOGI(TAG, "Device ID: %s", dc_device_id);

  if (dc_wifi_init_sta() != ESP_OK)
  {
    ESP_LOGE(TAG, "Wi-Fi init failed");
    return;
  }

  if (dc_mqtt_start() != ESP_OK)
  {
    ESP_LOGE(TAG, "MQTT start failed");
    return;
  }

  ESP_LOGI(TAG, "discusFW started");

  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}