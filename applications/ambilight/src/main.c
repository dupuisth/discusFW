#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_err.h>
#include <esp_log.h>
#include <discus/core/dc_core.h>
#include <discus/devices/ledstrip/dc_ledstrip.h>
#include <discus/devices/uart/dc_uart.h>
#include <discus/status/dc_status.h>
#include <discus_ambilight/dc_ambilight.h>
#include <discus_ambilight/transports/dc_ambilight_uart.h>

void app_main(void)
{
  dc_init();

  dc_ledstrip_t ledstrip;
  if (dc_ledstrip_create(4, 1, &ledstrip) != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    return;
  }

  dc_uart_config_t uart_config = {.port = UART_NUM_1,
      .baud_rate = 115200,
      .tx_buffer_size = 0,
      .rx_buffer_size = 1024,
      .event_queue_size = 0,
      .event_queue = NULL,
      .gpio_tx = 24,
      .gpio_rx = 23,
      .gpio_rts = UART_PIN_NO_CHANGE,
      .gpio_cts = UART_PIN_NO_CHANGE,
      .gpio_dsr = UART_PIN_NO_CHANGE,
      .gpio_dtr = UART_PIN_NO_CHANGE};

  dc_ambilight_transport_handler_t transport;
  if (dc_ambilight_transport_uart_create(&uart_config, &ledstrip, &transport) != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    return;
  }

  dc_ambilight_transport_uart_set_single_pixel(&transport, (dc_rgb8_t){.r = 255, .g = 0, .b = 5}, 0);
  while (true)
  {
    vTaskDelay(pdMS_TO_TICKS(100));
    if (dc_ambilight_transport_uart_poll(&transport) != ESP_OK)
    {
      dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
      vTaskDelay(pdMS_TO_TICKS(5000));
      return;
    }
  }
}
