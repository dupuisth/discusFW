#include <esp_check.h>
#include <esp_err.h>
#include <discus/devices/uart/dc_uart.h>
#include <discus/status/dc_status.h>

static const char* TAG = "dc_uart";

esp_err_t dc_uart_init(dc_uart_config_t* config, dc_uart_device_t* device)
{
  dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_INIT, 0);

  device->enabled = false;
  device->port = config->port;

  esp_err_t err;
  err = uart_driver_install(config->port, config->rx_buffer_size, config->tx_buffer_size, config->event_queue_size, config->event_queue, 0);
  if (err != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to install UART driver");
  }

  uart_config_t uart_config = {
      .baud_rate = config->baud_rate,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
      .rx_flow_ctrl_thresh = 122,
  };

  err = uart_param_config(config->port, &uart_config);
  if (err != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to install configure UART");
  }

  err = uart_set_pin(config->port, config->gpio_tx, config->gpio_rx, config->gpio_rts, config->gpio_cts);
  if (err != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set pin");
  }

  dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_OK, 0);
  device->enabled = true;
  return err;
}

esp_err_t dc_uart_update_baud_rate(dc_uart_device_t* device, uint32_t baudrate)
{
  if (!device->enabled)
  {
    ESP_LOGE(TAG, "Trying to update the UART baud rate but the device is not enabled");
    return ESP_ERR_INVALID_STATE;
  }

  esp_err_t err = uart_set_baudrate(device->port, baudrate);
  if (err != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to update baudrate");
  }

  return err;
}

esp_err_t dc_uart_write(dc_uart_device_t* device, const void* data, size_t size, int* bytes_wrote)
{
  if (!device->enabled)
  {
    ESP_LOGE(TAG, "Trying to write on UART but the device is not enabled");
    return ESP_ERR_INVALID_STATE;
  }

  dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_OK, DC_STATUS_CONNECTIVITY_WRITING);
  int res = uart_write_bytes(device->port, data, size);
  if (res < 0)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, DC_STATUS_CONNECTIVITY_ERROR);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Failed to write bytes");
  }

  if (bytes_wrote != NULL)
  {
    *bytes_wrote = res;
  }

  dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_OK, 0);
  return ESP_OK;
}

esp_err_t dc_uart_read_size(dc_uart_device_t* device, int* bytes)
{
  if (!device->enabled)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Trying to read on UART but the device is not enabled");
  }

  esp_err_t err = uart_get_buffered_data_len(device->port, (size_t*)bytes);
  if (err != ESP_OK)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, DC_STATUS_CONNECTIVITY_ERROR);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to get buffered data length");
  }
  return ESP_OK;
}

esp_err_t dc_uart_read(dc_uart_device_t* device, void* data, size_t size_bytes, int* bytes_read, TickType_t ticks_to_wait)
{
  if (!device->enabled)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, 0);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Trying to read on UART but the device is not enabled");
  }

  dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_OK, DC_STATUS_CONNECTIVITY_READING);
  // Read the bytes
  int bytes_read_val = uart_read_bytes(device->port, data, size_bytes, ticks_to_wait);
  if (bytes_read_val < 0)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_ERROR, DC_STATUS_CONNECTIVITY_ERROR);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Failed to read bytes");
  }

  if (bytes_read != NULL)
  {
    *bytes_read = bytes_read_val;
  }

  dc_status_manager_set(DC_STATUS_DOMAIN_UART, DC_STATUS_LEVEL_SYSTEM_OK, 0);
  return ESP_OK;
}