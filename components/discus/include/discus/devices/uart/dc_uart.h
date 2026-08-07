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

esp_err_t dc_uart_init(dc_uart_config_t* config, dc_uart_device_t* device);

esp_err_t dc_uart_write(dc_uart_device_t* device, const void* data, size_t size_bytes, int* bytes_wrote);
esp_err_t dc_uart_read_size(dc_uart_device_t* device, int* bytes);
esp_err_t dc_uart_read(dc_uart_device_t* device, void* data, size_t size_bytes, int* bytes_read, TickType_t ticks_to_wait);

// Quick debug
#if false
#define DC_UART_DEBUG_WRITE_MSG(device, buffer_size, message, ...)                                                                                   \
  {                                                                                                                                                  \
    char uart_debug_buffer[buffer_size];                                                                                                             \
    snprintf(uart_debug_buffer, (size_t)buffer_size, message, __VA_ARGS__);                                                                          \
    dc_uart_write(device, uart_debug_buffer, strnlen(uart_debug_buffer, buffer_size), NULL);                                                         \
  }
#else
#define DC_UART_DEBUG_WRITE_MSG(device, buffer_size, message, ...)
#endif

#endif