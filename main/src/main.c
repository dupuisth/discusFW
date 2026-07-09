#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <discus/core/dc_core.h>
#include <discus/status/dc_status.h>

void app_main(void)
{
  dc_init();

  ESP_LOGI("app", "Hello world !");

  while (true)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_ZIGBEE, DC_STATUS_LEVEL_SYSTEM_INIT, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));
    dc_status_manager_set(DC_STATUS_DOMAIN_ZIGBEE, DC_STATUS_LEVEL_SYSTEM_OK, 0);
    ESP_LOGI("app", "Hi again, ticking!");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
