#include <esp_check.h>
#include <esp_err.h>
#include <discus/checksum/dc_checksum.h>
#include <discus/status/dc_status.h>
#include <discus_ambilight/transports/dc_ambilight_uart.h>

static const char* TAG = "dc_ambilight_uart";

esp_err_t dc_ambilight_uart_create(dc_uart_config_t* uart_config, dc_ledstrip_t* ledstrip, dc_ambilight_uart_data_t* transport)
{
  // Prepare transport
  transport->transport = DC_ambilight_uart;
  transport->transport_data = NULL;

  // Initialize UART
  dc_uart_device_t uart_device;
  dc_uart_init(uart_config, &uart_device);

  // Create the data pointer and initialize
  dc_ambilight_uart_data_t* data = malloc(sizeof(dc_ambilight_uart_data_t));
  if (data == NULL)
  {
    ESP_RETURN_ON_ERROR(ESP_ERR_NO_MEM, TAG, "Could not allocate the UART transport data");
  }

  data->uart_device = uart_device;
  data->ledstrip = ledstrip;

  // Initialize the transport
  transport->transport_data = data;

  return ESP_OK;
}

esp_err_t dc_ambilight_uart_free(dc_ambilight_uart_data_t* transport)
{
  if (transport->transport != DC_ambilight_uart)
  {
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Trying to free UART transport but the given transport is '%d'", transport->transport);
  }

  if (transport->transport_data == NULL)
  {
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Trying to free UART transport but the data pointer is NULL");
  }

  free(transport->transport_data);
  transport->transport_data = NULL;
  return ESP_OK;
}

void dc_ambilight_uart_set_pixels(dc_ambilight_uart_data_t* transport, dc_rgb8_t* pixels, uint32_t pixel_count)
{
  dc_ambilight_uart_data_t* data = (dc_ambilight_uart_data_t*)transport->transport_data;

  if (pixel_count != data->ledstrip->pixel_count)
  {
    ESP_LOGE(TAG, "Mismatch pixel count when settings all pixels at once (actual: %d, received: %d)", data->ledstrip->pixel_count, pixel_count);
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
    return;
  }

  for (uint32_t i = 0; i < pixel_count; i++)
  {
    dc_ledstrip_set_pixel(data->ledstrip, i, pixels[i]);
  }
  dc_ledstrip_flush(data->ledstrip);
}

void dc_ambilight_uart_set_single_pixel(dc_ambilight_uart_data_t* transport, dc_rgb8_t pixel, uint32_t pixel_index)
{
  if (pixel_index >= transport->ledstrip->pixel_count)
  {
    ESP_LOGE(
        TAG, "Trying to set a pixel that is outside of the ledstrip count (index: %d, count: %d)", pixel_index, transport->ledstrip->pixel_count);
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
    return;
  }

  dc_ledstrip_set_pixel(transport->ledstrip, pixel_index, pixel);
}

esp_err_t dc_ambilight_uart_run(dc_ambilight_transport_handler_t* transport)
{
  dc_ambilight_uart_data_t* data = (dc_ambilight_uart_data_t*)transport->transport_data;

  const size_t SizeOfFrame = sizeof(dc_ambilight_uart_frame_t);
  dc_ambilight_uart_frame_t frame;

  while (true)
  {
    size_t received = 0;

    while (received < SizeOfFrame)
    {
      int length = 0;
      esp_err_t err = dc_uart_read(&data->uart_device, ((uint8_t*)&frame) + received, SizeOfFrame - received, &length, portMAX_DELAY);

      if (err != ESP_OK || length <= 0)
      {
        dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
        return err;
      }

      received += (size_t)length;

      if (received >= offsetof(dc_ambilight_uart_frame_t, payload) && frame.magic != DC_ambilight_uart_MAGIC)
      {
        received = 0;
      }
    }

    if (!dc_ambilight_uart_validate_frame(&frame))
    {
      continue;
    }

    esp_err_t err = dc_ambilight_uart_handle_frame(transport, &frame);
    if (err != ESP_OK)
    {
      return err;
    }
  }
}

esp_err_t dc_ambilight_uart_handle_frame(dc_ambilight_transport_handler_t* transport, dc_ambilight_uart_frame_t* frame)
{
  dc_ambilight_uart_data_t* data = (dc_ambilight_uart_data_t*)transport->transport_data;

  // Debug log
  char tx_buffer[64];
  sprintf(tx_buffer, "Frame: [%d] %d %d %d %d\r\n", frame->command, frame->payload[0], frame->payload[1], frame->payload[2], frame->payload[3]);
  dc_uart_write(&data->uart_device, (void*)tx_buffer, strlen(tx_buffer), NULL);

  if (frame->command == DC_ambilight_uart_COMMAND_SET_PIXEL)
  {
    uint8_t index = frame->payload[0];
    dc_rgb8_t color = {.r = frame->payload[1], .g = frame->payload[2], .b = frame->payload[3]};
    return dc_ledstrip_set_pixel(data->ledstrip, index, color);
  }
  else if (frame->command == DC_ambilight_uart_COMMAND_FLUSH)
  {
    return dc_ledstrip_flush(data->ledstrip);
  }
  else
  {
    return ESP_ERR_INVALID_ARG;
  }

  return ESP_OK;
}

bool dc_ambilight_uart_validate_frame(const dc_ambilight_uart_frame_t* frame)
{
  if (frame->magic != DC_ambilight_uart_MAGIC)
  {
    return false;
  }

  return frame->checksum ==
         dc_crc16_ccitt(
             (const uint8_t*)frame, offsetof(dc_ambilight_uart_frame_t, checksum), DC_ambilight_uart_CRC_INITIAL, DC_ambilight_uart_CRC_POLY);
}