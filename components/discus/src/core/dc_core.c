#include <discus/core/dc_core.h>
#include <discus/core/dc_indicator.h>
#include <esp_log.h>
#include <nvs_flash.h>

const char* TAG = "discus_core";

dc_indicator_t dc_indicator_global;
dc_indicator_t dc_indicator_connectivity;

esp_err_t dc_init_global_indicator()
{
  esp_err_t err = ESP_OK;
#if CONFIG_DC_INDICATOR_GLOBAL_TYPE_RMT
  dc_indicator_global.enabled = true;
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_create_rmt(&dc_indicator_global, CONFIG_DC_INDICATOR_GLOBAL_RMT_GPIO));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }

  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_initialize(&dc_indicator_global));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }
#elif CONFIG_DC_INDICATOR_GLOBAL_TYPE_RGB
  dc_global_indicator.enabled = true;
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_create_rgb(
      &dc_indicator_global, CONFIG_DC_INDICATOR_GLOBAL_RGB_GPIO_R, CONFIG_DC_INDICATOR_GLOBAL_RGB_GPIO_G, CONFIG_DC_INDICATOR_GLOBAL_RGB_GPIO_B));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_initialize(&dc_indicator_global));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }
#else
  dc_global_indicator.enabled = false;
#endif
  return err;
}

esp_err_t dc_init_connectivity_indicator()
{
  esp_err_t err = ESP_OK;
#if CONFIG_DC_INDICATOR_CONNECTIVITY_TYPE_RMT
  dc_indicator_connectivity.enabled = true;
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_create_rmt(&dc_indicator_connectivity, CONFIG_DC_INDICATOR_CONNECTIVITY_RMT_GPIO));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }

  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_initialize(&dc_indicator_connectivity));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }
#elif CONFIG_DC_INDICATOR_CONNECTIVITY_TYPE_RGB
  dc_indicator_connectivity.enabled = true;
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_create_rgb(&dc_indicator_connectivity,
      CONFIG_DC_INDICATOR_CONNECTIVITY_RGB_GPIO_R,
      CONFIG_DC_INDICATOR_CONNECTIVITY_RGB_GPIO_G,
      CONFIG_DC_INDICATOR_CONNECTIVITY_RGB_GPIO_B));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_initialize(&dc_indicator_connectivity));
  if (err)
  {
    dc_indicator_global.enabled = false;
    return err;
  }
#else
  dc_connectivity_indicator.enabled = false;
#endif

  return err;
}

esp_err_t dc_init(void)
{
  esp_err_t err = ESP_OK;
  ESP_LOGI(TAG, "Initialize discus");

  // Don't handle error as they are not critical here
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_init_global_indicator());
  if (err)
  {
    ESP_LOGE(TAG, "Failed to initialize the global indicator");
  }
  dc_indicator_set_state(&dc_indicator_global, DC_INDICATOR_STATE_SETUP);

  // Idem
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_init_connectivity_indicator());
  if (err)
  {
    ESP_LOGE(TAG, "Failed to initialize the connectivity indicator");

    // Just indicate there was an error
    dc_indicator_set_state(&dc_indicator_global, DC_INDICATOR_STATE_ERROR);
    vTaskDelay(pdMS_TO_TICKS(2000));
  }

  err = ESP_ERROR_CHECK_WITHOUT_ABORT(nvs_flash_init());
  if (err)
  {
    dc_indicator_set_state(&dc_indicator_global, DC_INDICATOR_STATE_FATAL);
    return err;
  }

  dc_indicator_set_state(&dc_indicator_global, DC_INDICATOR_STATE_IDLE);
  return ESP_OK;
}
