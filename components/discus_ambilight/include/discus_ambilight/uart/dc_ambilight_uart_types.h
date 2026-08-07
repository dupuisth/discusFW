#ifndef DC_AMIBLIGHT_UART_TYPES_H
#define DC_AMIBLIGHT_UART_TYPES_H

#include <discus/devices/ledstrip/dc_ledstrip.h>
#include <discus/devices/uart/dc_uart.h>
#include <discus_ambilight/dc_ambilight.h>

#define DC_AMBILIGHT_UART_MAGIC 0x43U
#define DC_AMBILIGHT_UART_CRC_INITIAL 0xFFFFU
#define DC_AMBILIGHT_UART_CRC_POLY 0X1021U

#define DC_AMBILIGHT_UART_COMMAND_CONFIGURE 0
#define DC_AMBILIGHT_UART_COMMAND_SET_PIXEL 1
#define DC_AMBILIGHT_UART_COMMAND_FLUSH 2

// Data stored in the transport data pointer
typedef struct
{
  dc_uart_device_t uart_device;

  dc_rgb8_t* rx_pixels;
  uint32_t pixel_count;

  dc_ambilight_shared_data_t* shared;
} dc_ambilight_uart_data_t;

typedef struct
{
  uint8_t magic;
  uint8_t command;
  // CONFIGURE: [LED COUNT: 8] [NOT USED: 24]
  // SET_PIXEL: [INDEX: 8] [R: 8] [G: 8] [B: 8]
  // FLUSH: [NOT USED: 32]
  uint8_t payload[4];
  uint16_t checksum;
} dc_ambilight_uart_frame_t;

// Checking that it match the expected format
ESP_STATIC_ASSERT(sizeof(dc_ambilight_uart_frame_t) == 8, "Expected 8 bytes for dc_ambilight_uart_frame_t");

#endif