#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <discus/core/dc_core.h>
#include <discus/devices/ledstrip/dc_ledstrip.h>
#include <discus/status/dc_status.h>

void app_main(void)
{
  dc_init();

  ESP_LOGI("app", "Hello world !");

  dc_ledstrip_t ledstrip;
  dc_ledstrip_create(4, 1, &ledstrip);

  while (true)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_ZIGBEE, DC_STATUS_LEVEL_SYSTEM_INIT, 0);
    dc_ledstrip_fill_pixels(&ledstrip, (dc_rgb8_t){255, 0, 0});
    dc_ledstrip_flush(&ledstrip);
    vTaskDelay(pdMS_TO_TICKS(1000));
    dc_status_manager_set(DC_STATUS_DOMAIN_ZIGBEE, DC_STATUS_LEVEL_SYSTEM_OK, 0);
    dc_ledstrip_fill_pixels(&ledstrip, (dc_rgb8_t){0, 255, 0});
    dc_ledstrip_flush(&ledstrip);
    ESP_LOGI("app", "Hi again, ticking!");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
