#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <discus/core/dc_core.h>
#include <discus/devices/ledstrip/dc_ledstrip.h>
#include <discus/devices/uart/dc_uart.h>
#include <discus/status/dc_status.h>

void app_main(void)
{
  dc_init();
  ESP_LOGI("app", "Hello world !");
  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    ESP_LOGI("app", "Hi again, ticking!");
  }
}
