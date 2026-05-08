#include <discus/core/dc_core.h>
#include <discus_zigbee/dc_zigbee.h>
#include <discus_zigbee/devices/dc_zigbee_ledstrip.h>
#include <esp_err.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

static dc_zigbee_endpoint_t endpoints[1];
static dc_zigbee_device_t device;
static dc_zigbee_config_t zb_config;

void app_main(void)
{
  dc_init();

  ESP_ERROR_CHECK(dc_zigbee_ledstrip_create(10, &endpoints[0]));
  device = (dc_zigbee_device_t){.manufacturer = "Discus", .model = "ledstrip", .endpoints = endpoints, .endpoint_count = 1};
  zb_config = (dc_zigbee_config_t){.device = &device};
  ESP_ERROR_CHECK(dc_zigbee_init(&zb_config));

  ESP_LOGI("app", "Hello world !");

  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI("app", "Hi again, ticking!");
  }
}
