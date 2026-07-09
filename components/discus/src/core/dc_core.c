#include <nvs_flash.h>
#include <esp_log.h>
#include <discus/core/dc_core.h>
#include <discus/indicator/dc_led_indicator.h>
#include <discus/status/dc_status.h>

const char* TAG = "discus_core";

esp_err_t dc_init(void)
{
  esp_err_t err = ESP_OK;
  ESP_LOGI(TAG, "Initialize discus");
  dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_INIT, 0);

  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_led_indicator_init());
  if (err)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);

    return err;
  }

  err = ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_init());
  if (err)
  {
    dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_FATAL, 0);
    return err;
  }
  dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_OK, 0);

  ESP_LOGI(TAG, "Initialized!");
  return ESP_OK;
}
