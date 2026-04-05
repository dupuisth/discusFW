#include "dc_core.h"

char dc_device_id[DC_DEVICE_ID_LEN] = {0};

void dc_get_device_id(char* out, size_t out_len)
{
  if (out == NULL || out_len == 0)
  {
    return;
  }

  uint8_t mac[6] = {0};
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  snprintf(out, out_len, "esp32%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

esp_err_t dc_core_init(void)
{
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }

  if (err != ESP_OK)
  {
    return err;
  }

  dc_get_device_id(dc_device_id, sizeof(dc_device_id));
  return ESP_OK;
}