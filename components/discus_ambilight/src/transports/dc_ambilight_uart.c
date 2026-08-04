#include <esp_check.h>
#include <esp_err.h>
#include <discus/status/dc_status.h>
#include <discus_ambilight/transports/dc_ambilight_uart.h>

static const char* TAG = "dc_ambilight_uart";

esp_err_t dc_ambilight_transport_uart_create(dc_uart_config_t* uart_config, dc_ledstrip_t* ledstrip, dc_ambilight_transport_handler_t* transport)
{
  // Prepare transport
  transport->transport = DC_AMBILIGHT_TRANSPORT_UART;
  transport->transport_data = NULL;
  transport->set_pixels_handler = NULL;
  transport->set_single_pixel_handler = NULL;

  // Initialize UART
  dc_uart_device_t uart_device;
  dc_uart_init(uart_config, &uart_device);

  // Create the data pointer and initialize
  dc_ambilight_transport_uart_data_t* data = malloc(sizeof(dc_ambilight_transport_uart_data_t));
  if (data == NULL)
  {
    ESP_RETURN_ON_ERROR(ESP_ERR_NO_MEM, TAG, "Could not allocate the UART transport data");
  }

  data->uart_device = uart_device;
  data->ledstrip = ledstrip;

  // Initialize the transport
  transport->set_pixels_handler = dc_ambilight_transport_uart_set_pixels;
  transport->set_single_pixel_handler = dc_ambilight_transport_uart_set_single_pixel;
  transport->transport_data = data;

  return ESP_OK;
}

esp_err_t dc_ambilight_transport_uart_free(dc_ambilight_transport_handler_t* transport)
{
  if (transport->transport != DC_AMBILIGHT_TRANSPORT_UART)
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

void dc_ambilight_transport_uart_set_pixels(dc_ambilight_transport_handler_t* transport, dc_rgb8_t* pixels, uint32_t pixel_count)
{
  dc_ambilight_transport_uart_data_t* data = (dc_ambilight_transport_uart_data_t*)transport->transport_data;

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

void dc_ambilight_transport_uart_set_single_pixel(dc_ambilight_transport_handler_t* transport, dc_rgb8_t pixel, uint32_t pixel_index)
{
  dc_ambilight_transport_uart_data_t* data = (dc_ambilight_transport_uart_data_t*)transport->transport_data;

  if (pixel_index >= data->ledstrip->pixel_count)
  {
    ESP_LOGE(TAG, "Trying to set a pixel that is outside of the ledstrip count (index: %d, count: %d)", pixel_index, data->ledstrip->pixel_count);
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
    return;
  }

  dc_ledstrip_set_pixel(data->ledstrip, pixel_index, pixel);
  dc_ledstrip_flush(data->ledstrip);
}

esp_err_t dc_ambilight_transport_uart_poll(dc_ambilight_transport_handler_t* transport)
{
  dc_ambilight_transport_uart_data_t* data = (dc_ambilight_transport_uart_data_t*)transport->transport_data;

  char buffer;
  int rx_length = 10;
  ESP_ERROR_CHECK(dc_uart_read_size(&data->uart_device, &rx_length));

  char tx_buffer[64];

  sprintf(tx_buffer, "%d\r\n", rx_length);
  dc_uart_write(&data->uart_device, tx_buffer, strlen(tx_buffer), NULL);

  for (int i = 0; i < rx_length; i++)
  {
    dc_uart_read(&data->uart_device, &buffer, 1, NULL);
    ESP_LOGI(TAG, "%c", buffer);

    if (buffer == '0')
    {
      transport->set_single_pixel_handler(transport, (dc_rgb8_t){.r = 64, .g = 128, .b = 64}, 0);
    }
    else if (buffer == '1')
    {
      transport->set_single_pixel_handler(transport, (dc_rgb8_t){.r = 16, .g = 16, .b = 64}, 0);
    }
    else if (buffer == '2')
    {
      transport->set_single_pixel_handler(transport, (dc_rgb8_t){.r = 128, .g = 16, .b = 64}, 0);
    }
    else if (buffer == '3')
    {
      transport->set_single_pixel_handler(transport, (dc_rgb8_t){.r = 0, .g = 0, .b = 64}, 0);
    }
    else
    {
      transport->set_single_pixel_handler(transport, (dc_rgb8_t){.r = 0, .g = 0, .b = 128}, 0);
    }
  }

  return ESP_OK;
}