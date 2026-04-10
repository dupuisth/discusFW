#include "driver/gpio.h"

#include <discusFW/indicator/dc_indicator.h>

#ifdef CONFIG_DC_INDICATOR_TYPE_GPIO_RGB

static esp_err_t dc_indicator_set_gpio_color(uint32_t r, uint32_t g, uint32_t b)
{
  esp_err_t err;

  err = gpio_set_level(CONFIG_DC_ESP_INDICATOR_GPIO_R, r);
  if (err != ESP_OK)
    return err;

  err = gpio_set_level(CONFIG_DC_ESP_INDICATOR_GPIO_G, g);
  if (err != ESP_OK)
    return err;

  err = gpio_set_level(CONFIG_DC_ESP_INDICATOR_GPIO_B, b);
  if (err != ESP_OK)
    return err;

  return ESP_OK;
}

esp_err_t dc_indicator_initialize(void)
{
  esp_err_t err;

  err = gpio_set_direction(CONFIG_DC_ESP_INDICATOR_GPIO_R, GPIO_MODE_OUTPUT);
  if (err != ESP_OK)
    return err;

  err = gpio_set_direction(CONFIG_DC_ESP_INDICATOR_GPIO_G, GPIO_MODE_OUTPUT);
  if (err != ESP_OK)
    return err;

  err = gpio_set_direction(CONFIG_DC_ESP_INDICATOR_GPIO_B, GPIO_MODE_OUTPUT);
  if (err != ESP_OK)
    return err;

  return dc_indicator_set_gpio_color(0, 0, 0);
}

esp_err_t dc_indicator_set_state(dc_indicator_state_id_t state)
{
  switch (state)
  {
  case DC_INDICATOR_STATE_OFF:
    return dc_indicator_set_gpio_color(0, 0, 0);

  case DC_INDICATOR_STATE_BOOTING:
    return dc_indicator_set_gpio_color(0, 0, 1);

  case DC_INDICATOR_STATE_CONNECTING:
    return dc_indicator_set_gpio_color(0, 1, 1);

  case DC_INDICATOR_STATE_OK:
    return dc_indicator_set_gpio_color(0, 1, 0);

  case DC_INDICATOR_STATE_WARNING:
    return dc_indicator_set_gpio_color(1, 1, 0);

  case DC_INDICATOR_STATE_ERROR:
    return dc_indicator_set_gpio_color(1, 0, 0);

  case DC_INDICATOR_STATE_FATAL:
    return dc_indicator_set_gpio_color(1, 1, 1);

  default:
    return dc_indicator_set_gpio_color(0, 0, 0);
  }
}

#endif
