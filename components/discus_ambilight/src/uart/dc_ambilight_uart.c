#include <esp_check.h>
#include <esp_err.h>
#include <discus/checksum/dc_checksum.h>
#include <discus/status/dc_status.h>
#include <discus_ambilight/uart/dc_ambilight_uart.h>

static const char* TAG = "dc_ambilight_uart";

esp_err_t dc_ambilight_uart_create(dc_uart_config_t* uart_config, dc_ambilight_shared_data_t* shared, dc_ambilight_uart_data_t* transport)
{
  // Initialize UART
  dc_uart_device_t uart_device;
  esp_err_t err = dc_uart_init(uart_config, &uart_device);
  ESP_RETURN_ON_ERROR(err, TAG, "Failed to intialize UART device");

  // Populate transport
  memset(transport, 0, sizeof(dc_ambilight_uart_data_t));
  transport->uart_device = uart_device;
  transport->shared = shared;

  return ESP_OK;
}

esp_err_t dc_ambilight_uart_free(dc_ambilight_uart_data_t* transport)
{
  if (transport->rx_pixels != NULL)
  {
    free(transport->rx_pixels);
    transport->pixel_count = 0;
  }

  return ESP_OK;
}

esp_err_t dc_ambilight_uart_run(dc_ambilight_uart_data_t* transport)
{
  dc_ambilight_uart_frame_t frame;

  while (true)
  {
    if (dc_ambilight_uart_read_frame(transport, &frame))
    {
      continue;
    }

    if (!dc_ambilight_uart_validate_frame(transport, &frame))
    {
      continue;
    }

    esp_err_t err = dc_ambilight_uart_handle_frame(transport, &frame);
    if (err != ESP_OK)
    {
      DC_UART_DEBUG_WRITE_MSG(&transport->uart_device, 64, "Something went wrong handling the frame (%d)..\r\n", err);
      return err;
    }
  }
}

esp_err_t dc_ambilight_uart_read_frame(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame)
{
  const size_t SizeOfFrame = sizeof(dc_ambilight_uart_frame_t);

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
  frame->magic = magic_byte;

  // Then read the whole frame
  size_t received = 1;
  while (received < SizeOfFrame)
  {
    // Reset read length
    length = 0;
    esp_err_t err = dc_uart_read(&transport->uart_device, ((uint8_t*)frame) + received, SizeOfFrame - received, &length, portMAX_DELAY);

    if (err != ESP_OK)
    {
      dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
      return err;
    }

    if (length > 0)
    {
      received += (size_t)length;
    }
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

  if (frame->command == DC_AMBILIGHT_UART_COMMAND_CONFIGURE)
  {
    return dc_ambilight_uart_handle_command_configure(transport, frame);
  }
  else if (frame->command == DC_AMBILIGHT_UART_COMMAND_SET_PIXEL)
  {
    return dc_ambilight_uart_handle_command_set_pixel(transport, frame);
  }
  else if (frame->command == DC_AMBILIGHT_UART_COMMAND_FLUSH)
  {
    return dc_ambilight_uart_handle_command_flush(transport, frame);
  }
  else
  {
    return ESP_ERR_INVALID_ARG;
  }
  return ESP_OK;
}

///////////////////////////////////////////
// Commands
///////////////////////////////////////////
esp_err_t dc_ambilight_uart_handle_command_configure(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame)
{
  uint8_t pixel_count = frame->payload[0];
  DC_UART_DEBUG_WRITE_MSG(&transport->uart_device, 64, "Configure pixel_count: %d\r\n", pixel_count);

  if (transport->pixel_count == pixel_count)
  {
    return ESP_OK;
  }

  if (transport->rx_pixels == NULL)
  {
    transport->rx_pixels = malloc(sizeof(dc_rgb8_t) * pixel_count);
  }
  else
  {
    transport->rx_pixels = realloc(transport->rx_pixels, sizeof(dc_rgb8_t) * pixel_count);
  }

  if (transport->rx_pixels == NULL)
  {
    ESP_RETURN_ON_ERROR(ESP_ERR_NO_MEM, TAG, "Failed to allocate memory for rx_pixels");
  }

  transport->pixel_count = pixel_count;
  return dc_ambilight_shared_data_reconfigure(transport->shared, pixel_count);
}

esp_err_t dc_ambilight_uart_handle_command_set_pixel(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame)
{
  uint8_t index = frame->payload[0];
  dc_rgb8_t color = {.r = frame->payload[1], .g = frame->payload[2], .b = frame->payload[3]};
  if (index < transport->pixel_count)
  {
    transport->rx_pixels[index] = color;
  }
  return ESP_OK;
}

esp_err_t dc_ambilight_uart_handle_command_flush(dc_ambilight_uart_data_t* transport, dc_ambilight_uart_frame_t* frame)
{
  return dc_ambilight_shared_data_publish(transport->shared, transport->rx_pixels, transport->pixel_count);
}