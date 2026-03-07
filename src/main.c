/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "dc_core.h"
#include "dc_mqtt.h"
#include "dc_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include <string.h>

static void echo_handler(const char* topic, const uint8_t* data, size_t data_len, void* user_ctx)
{
  char snd_topic[DC_MQTT_MAX_TOPIC_LEN];
  snprintf(snd_topic, sizeof(snd_topic), "device/%s/echo/response", dc_device_id);

  dc_mqtt_publish(snd_topic, data, data_len, 0, 0, 0);
}

void app_main(void)
{
  dc_get_device_id(dc_device_id, 32);

  // Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  if (dc_wifi_init_sta() == ESP_OK)
  {
    // Testing
    char topic[DC_MQTT_MAX_TOPIC_LEN];
    snprintf(topic, sizeof(topic), "device/%s/echo", dc_device_id);
    dc_mqtt_register_topic_handler(topic, echo_handler, NULL);

    if (dc_mqtt_start() != ESP_OK)
    {
      ESP_LOGE(TAG, "Failed to start mqtt");
    }
  }
  else
  {
    ESP_LOGE(TAG, "Skipping mqtt_start because WiFi is not connected");
  }

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
