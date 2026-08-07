#ifndef DC_AMBILIGHT_UART_H
#define DC_AMBILIGHT_UART_H

#include <discus/devices/ledstrip/dc_ledstrip.h>
#include <discus/devices/uart/dc_uart.h>
#include <discus_ambilight/uart/dc_ambilight_uart_types.h>

/// @brief Create a transport with the given UART config. The UART device must not be already initialized, it will be done here.
/// @param uart_config UART device config. Will be initialized here
/// @param ledstrip The ledstrip handle, must be created using dc_ledstrip_create
/// @param transport The result transport
esp_err_t dc_ambilight_uart_create(dc_uart_config_t* uart_config, dc_ambilight_shared_data_t* shared_data, dc_ambilight_uart_data_t* transport);
esp_err_t dc_ambilight_uart_free(dc_ambilight_uart_data_t* transport);

esp_err_t dc_ambilight_uart_run(dc_ambilight_uart_data_t* transport);

esp_err_t dc_ambilight_uart_read_frame(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame);
bool dc_ambilight_uart_validate_frame(dc_ambilight_uart_data_t* transport, const dc_ambilight_uart_frame_t* frame);
esp_err_t dc_ambilight_uart_handle_frame(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame);

///////////////////////////////////////////
// Commands
///////////////////////////////////////////
esp_err_t dc_ambilight_uart_handle_command_configure(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame);
esp_err_t dc_ambilight_uart_handle_command_set_pixel(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame);
esp_err_t dc_ambilight_uart_handle_command_flush(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame);

#endif