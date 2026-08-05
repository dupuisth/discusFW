#include <esp_check.h>
#include <esp_err.h>
#include <discus/checksum/dc_checksum.h>
#include <discus/status/dc_status.h>
#include <discus_ambilight/uart/dc_ambilight_uart.h>

static const char* TAG = "dc_ambilight_uart";

esp_err_t dc_ambilight_uart_create(dc_uart_config_t* uart_config, dc_ledstrip_t* ledstrip, dc_ambilight_uart_data_t* transport)
{
  // Initialize UART
  dc_uart_device_t uart_device;
  dc_uart_init(uart_config, &uart_device);

  // Populate transport
  transport->uart_device = uart_device;
  transport->ledstrip = ledstrip;

  return ESP_OK;
}

esp_err_t dc_ambilight_uart_free(dc_ambilight_uart_data_t* transport)
{
  return ESP_OK;
}

void dc_ambilight_uart_set_pixels(dc_ambilight_uart_data_t* transport, dc_rgb8_t* pixels, uint32_t pixel_count)
{
  if (pixel_count != transport->ledstrip->pixel_count)
  {
    ESP_LOGE(TAG, "Mismatch pixel count when settings all pixels at once (actual: %d, received: %d)", transport->ledstrip->pixel_count, pixel_count);
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
    return;
  }

  for (uint32_t i = 0; i < pixel_count; i++)
  {
    dc_ledstrip_set_pixel(transport->ledstrip, i, pixels[i]);
  }
  dc_ledstrip_flush(transport->ledstrip);
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

esp_err_t dc_ambilight_uart_run(dc_ambilight_uart_data_t* transport)
{
  const size_t SizeOfFrame = sizeof(dc_ambilight_uart_frame_t);
  dc_ambilight_uart_frame_t frame;

  while (true)
  {
    // Search byte per byte for the beginning of the frame
    int length = 0;
    uint8_t magic_byte;
    do
    {
      esp_err_t err = dc_uart_read(&transport->uart_device, &magic_byte, 1, &length, portMAX_DELAY);
      if (err != ESP_OK || length <= 0)
      {
        dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
        return err;
      }
    } while (length != 1 || magic_byte != DC_AMBILIGHT_UART_MAGIC);
    frame.magic = magic_byte;

    // Then read the whole frame
    size_t received = 1;
    while (received < SizeOfFrame)
    {
      // Reset read length
      length = 0;
      esp_err_t err = dc_uart_read(&transport->uart_device, ((uint8_t*)&frame) + received, SizeOfFrame - received, &length, portMAX_DELAY);

      if (err != ESP_OK)
      {
        dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
        return err;
      }

      if (length > 0)
      {
        received += (size_t)length;
      }

      DC_UART_DEBUG_WRITE_MSG(&transport->uart_device, 64, "\r\nCurrent frame: %d\r\n", (uint8_t)received);
    }

    if (!dc_ambilight_uart_validate_frame(transport, &frame))
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

esp_err_t dc_ambilight_uart_handle_frame(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame)
{

  // Debug log
  DC_UART_DEBUG_WRITE_MSG(&transport->uart_device,
      64,
      "Frame: [%d] %d %d %d %d\r\n",
      frame->command,
      frame->payload[0],
      frame->payload[1],
      frame->payload[2],
      frame->payload[3]);

  if (frame->command == DC_AMBILIGHT_UART_COMMAND_SET_PIXEL)
  {
    uint8_t index = frame->payload[0];
    dc_rgb8_t color = {.r = frame->payload[1], .g = frame->payload[2], .b = frame->payload[3]};
    return dc_ledstrip_set_pixel(transport->ledstrip, index, color);
  }
  else if (frame->command == DC_AMBILIGHT_UART_COMMAND_FLUSH)
  {
    return dc_ledstrip_flush(transport->ledstrip);
  }
  else
  {
    return ESP_ERR_INVALID_ARG;
  }

  return ESP_OK;
}

bool dc_ambilight_uart_validate_frame(dc_ambilight_uart_data_t* transport, const dc_ambilight_uart_frame_t* frame)
{
  if (frame->magic != DC_AMBILIGHT_UART_MAGIC)
  {
    DC_UART_DEBUG_WRITE_MSG(&transport->uart_device, 128, "Magic mismatch, expected %d got %d\r\n", DC_AMBILIGHT_UART_MAGIC, frame->magic);
    return false;
  }

  uint16_t checksum =
      dc_crc16_ccitt((const uint8_t*)frame, offsetof(dc_ambilight_uart_frame_t, checksum), DC_AMBILIGHT_UART_CRC_INITIAL, DC_AMBILIGHT_UART_CRC_POLY);

  if (frame->checksum != checksum)
  {
    DC_UART_DEBUG_WRITE_MSG(&transport->uart_device, 128, "Checksum mismatch, expected %x got %x\r\n", checksum, frame->checksum);
    return false;
  }
  return true;
}