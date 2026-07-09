#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <discus/core/dc_core.h>

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
