#ifndef DC_AMBILIGHT_UART_H
#define DC_AMBILIGHT_UART_H

#include <discus/devices/ledstrip/dc_ledstrip.h>
#include <discus/devices/uart/dc_uart.h>
#include <discus_ambilight/transports/dc_ambilight_transport.h>

// Data stored in the transport data pointer
typedef struct
{
  dc_uart_device_t uart_device;
  dc_ledstrip_t* ledstrip;
} dc_ambilight_transport_uart_data_t;

/// @brief Create a transport with the given UART config. The UART device must not be already initialized, it will be done here.
/// @param uart_config UART device config. Will be initialized here
/// @param ledstrip The ledstrip handle, must be created using dc_ledstrip_create
/// @param transport The result transport
esp_err_t dc_ambilight_transport_uart_create(dc_uart_config_t* uart_config, dc_ledstrip_t* ledstrip, dc_ambilight_transport_handler_t* transport);
esp_err_t dc_ambilight_transport_uart_free(dc_ambilight_transport_handler_t* transport);

esp_err_t dc_ambilight_transport_uart_poll(dc_ambilight_transport_handler_t* transport);

///////////////////////////////////////////
// Transport callbacks
///////////////////////////////////////////
void dc_ambilight_transport_uart_set_pixels(dc_ambilight_transport_handler_t* transport, dc_rgb8_t* pixels, uint32_t pixel_count);
void dc_ambilight_transport_uart_set_single_pixel(dc_ambilight_transport_handler_t* transport, dc_rgb8_t pixel, uint32_t pixel_index);

#endif