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

  QueueHandle_t uart_queue;
  dc_uart_config_t uart_config = {.port = UART_NUM_1,
      .baud_rate = 115200,
      .tx_buffer_size = 0,
      .rx_buffer_size = 1024,
      .event_queue_size = 10,
      .event_queue = &uart_queue,
      .gpio_tx = 24,
      .gpio_rx = 23,
      .gpio_rts = UART_PIN_NO_CHANGE,
      .gpio_cts = UART_PIN_NO_CHANGE,
      .gpio_dsr = UART_PIN_NO_CHANGE,
      .gpio_dtr = UART_PIN_NO_CHANGE};
  dc_uart_device_t uart_device;
  if (dc_uart_init(uart_config, &uart_device) != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
  }

  dc_ledstrip_t ledstrip;
  dc_ledstrip_create(4, 8, &ledstrip);

  while (true)
  {
    dc_ledstrip_fill_pixels(&ledstrip, (dc_rgb8_t){4, 0, 0});
    dc_ledstrip_flush(&ledstrip);
    vTaskDelay(pdMS_TO_TICKS(1000));
    dc_ledstrip_fill_pixels(&ledstrip, (dc_rgb8_t){0, 4, 4});
    dc_ledstrip_flush(&ledstrip);
    ESP_LOGI("app", "Hi again, ticking!");
    vTaskDelay(pdMS_TO_TICKS(1000));

    if (uart_device.enabled)
    {
      char* test_str = "Hello World!\r\n";
      if (dc_uart_write(&uart_device, (const char*)test_str, strlen(test_str), NULL) != ESP_OK)
      {
        dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
      }

      char read_buffer[512];
      int length = 0;
      if (dc_uart_read(&uart_device, read_buffer, 512, &length) == ESP_OK)
      {
        if (length > 0)
        {
          // Echo
          if (dc_uart_write(&uart_device, (const char*)read_buffer, length, NULL) != ESP_OK)
          {
            dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
          }
        }
      }
      else
      {
        dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
      }
    }

  }
}
