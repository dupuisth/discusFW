#ifndef DC_UART_H
#define DC_UART_H

#include <driver/uart.h>
#include <esp_err.h>

typedef struct
{
  uart_port_t port;
  int rx_buffer_size;
  int tx_buffer_size;
  int event_queue_size;
  QueueHandle_t* event_queue;

  int baud_rate;

  int gpio_rx;
  int gpio_tx;
  int gpio_rts;
  int gpio_cts;
  int gpio_dtr;
  int gpio_dsr;
} dc_uart_config_t;

typedef struct
{
  uart_port_t port;
  bool enabled;
} dc_uart_device_t;

esp_err_t dc_uart_init(dc_uart_config_t config, dc_uart_device_t* device);

esp_err_t dc_uart_write(dc_uart_device_t* device, const void* data, size_t size_bytes, int* bytes_wrote);

esp_err_t dc_uart_read(dc_uart_device_t* device, void* data, size_t size_bytes, int* bytes_read);

#endif