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
#include <discus_ambilight/ledstrip/dc_ambilight_ledstrip.h>
#include <discus_ambilight/uart/dc_ambilight_uart.h>
#include <discus_ambilight/uart/dc_ambilight_uart_types.h>

static const char* TAG = "main";

void app_main(void)
{
  if (dc_init() != ESP_OK)
  {
    vTaskDelay(pdMS_TO_TICKS(5000));
    exit(0);
  }
  vTaskDelay(pdMS_TO_TICKS(1000));

  dc_ambilight_shared_data_t shared;
  if (dc_ambilight_shared_data_init(&shared) != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    ESP_LOGE(TAG, "Failed to initialize shared data");
    vTaskDelay(pdMS_TO_TICKS(5000));
    exit(0);
  }

  dc_ledstrip_t ledstrip;
  memset(&ledstrip, 0, sizeof(dc_ledstrip_t));
  ledstrip.gpio = 2;

  // Devkit UART GPIO : 24 TX, 23 RX
  // Using an external UART chip (CP2102):
  dc_uart_config_t uart_config = {.port = UART_NUM_1,
      .baud_rate = 460800,
      .tx_buffer_size = 0,
      .rx_buffer_size = 1024,
      .event_queue_size = 0,
      .event_queue = NULL,
      .gpio_tx = 4,
      .gpio_rx = 5,
      .gpio_rts = UART_PIN_NO_CHANGE,
      .gpio_cts = UART_PIN_NO_CHANGE,
      .gpio_dsr = UART_PIN_NO_CHANGE,
      .gpio_dtr = UART_PIN_NO_CHANGE};

  dc_ambilight_uart_data_t transport;
  if (dc_ambilight_uart_create(&uart_config, &shared, &transport) != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to create UART");
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    vTaskDelay(pdMS_TO_TICKS(5000));
    return;
  }

  // Create the ledstrip task
  dc_ambilight_ledstrip_task_params_t params;
  params.ledstrip = &ledstrip;
  params.shared = &shared;
  xTaskCreate(dc_ambilight_ledstrip_task, "dc_ambilight_ledstrip_task", 2048, &params, 1, &shared.led_task);

  // // Run the UART loop
  dc_ambilight_uart_run(&transport);

  dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
  vTaskDelay(pdMS_TO_TICKS(5000));
  exit(0);
}
