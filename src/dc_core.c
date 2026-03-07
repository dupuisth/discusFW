#include "dc_core.h"

#include "esp_mac.h"

#include <stdio.h>

char dc_device_id[32] = "";

void dc_get_device_id(char* out, size_t out_len)
{
  uint8_t mac[6];
  esp_read_mac(mac, ESP_MAC_WIFI_STA);

  snprintf(out, out_len, "esp32%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}
