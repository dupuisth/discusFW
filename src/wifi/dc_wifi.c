#include <discusFW/wifi/dc_wifi.h>

static EventGroupHandle_t s_wifi_event_group = NULL;
static int s_retry_num = 0;

static void dc_wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
  (void)arg;

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
  {
    esp_wifi_connect();
    return;
  }

  if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    if (s_retry_num < DC_ESP_MAXIMUM_RETRY)
    {
      s_retry_num++;
      ESP_LOGW(TAG, "Wi-Fi disconnected, retry %d/%d", s_retry_num, DC_ESP_MAXIMUM_RETRY);
      esp_wifi_connect();
    }
    else
    {
      ESP_LOGE(TAG, "Wi-Fi connection failed");
      xEventGroupSetBits(s_wifi_event_group, DC_WIFI_FAIL_BIT);
    }
    return;
  }

  if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
  {
    ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
    s_retry_num = 0;

    ESP_LOGI(TAG, "Wi-Fi connected, IP: " IPSTR, IP2STR(&event->ip_info.ip));
    xEventGroupSetBits(s_wifi_event_group, DC_WIFI_CONNECTED_BIT);
  }
}

esp_err_t dc_wifi_init_sta(void)
{
  if (CONFIG_LOG_MAXIMUM_LEVEL > CONFIG_LOG_DEFAULT_LEVEL)
  {
    esp_log_level_set("wifi", CONFIG_LOG_MAXIMUM_LEVEL);
  }

  if (s_wifi_event_group == NULL)
  {
    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL)
    {
      return ESP_ERR_NO_MEM;
    }
  }

  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  esp_netif_create_default_wifi_sta();

  wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

  esp_event_handler_instance_t wifi_event_instance;
  esp_event_handler_instance_t ip_event_instance;

  ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &dc_wifi_event_handler, NULL, &wifi_event_instance));

  ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &dc_wifi_event_handler, NULL, &ip_event_instance));

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = DC_ESP_WIFI_SSID,
              .password = DC_ESP_WIFI_PASS,
              .threshold.authmode = DC_ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
              .sae_pwe_h2e = DC_ESP_WIFI_SAE_MODE,
              .sae_h2e_identifier = DC_ESP_WIFI_H2E_IDENTIFIER,
#ifdef CONFIG_DC_ESP_WIFI_WPA3_COMPATIBLE_SUPPORT
              .disable_wpa3_compatible_mode = 0,
#endif
          },
  };

  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGI(TAG, "Wi-Fi station started");

  EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group, DC_WIFI_CONNECTED_BIT | DC_WIFI_FAIL_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

  if (bits & DC_WIFI_CONNECTED_BIT)
  {
    ESP_LOGI(TAG, "Connected to SSID: %s", DC_ESP_WIFI_SSID);
    return ESP_OK;
  }

  if (bits & DC_WIFI_FAIL_BIT)
  {
    ESP_LOGE(TAG, "Failed to connect to SSID: %s", DC_ESP_WIFI_SSID);
    return ESP_FAIL;
  }

  return ESP_ERR_INVALID_STATE;
}