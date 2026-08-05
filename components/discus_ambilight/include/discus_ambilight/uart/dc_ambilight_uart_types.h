#ifndef DC_AMIBLIGHT_UART_TYPES_H
#define DC_AMIBLIGHT_UART_TYPES_H

#include <discus/devices/uart/dc_uart.h>

#define DC_ambilight_uart_MAGIC 0xA5U
#define DC_ambilight_uart_CRC_INITIAL 0xFFFFU
#define DC_ambilight_uart_CRC_POLY 0X1021U

#define DC_ambilight_uart_COMMAND_SET_PIXEL 0
#define DC_ambilight_uart_COMMAND_FLUSH 1

// Data stored in the transport data pointer
typedef struct
{
  dc_uart_device_t* uart_device;
  dc_ledstrip_t* ledstrip;
} dc_ambilight_uart_data_t;

typedef struct
{
  uint8_t magic;
  uint8_t command;
  // SET_PIXEL: [INDEX: 8] [R: 8] [G: 8] [B: 8]
  // FLUSH: [NOT USED: 32]
  uint8_t payload[4];
  uint16_t checksum;
} dc_ambilight_uart_frame_t;

// Checking that it match the expected format
ESP_STATIC_ASSERT(sizeof(dc_ambilight_uart_frame_t) == 8, "Expected 8 bytes for dc_ambilight_uart_frame_t");

#endif