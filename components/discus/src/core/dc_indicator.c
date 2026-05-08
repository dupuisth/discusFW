#include <discus/core/dc_indicator.h>
#include <driver/gpio.h>

esp_err_t dc_indicator_create_rmt(dc_indicator_t* indicator, uint8_t rmt_gpio)
{
  indicator->is_rmt = true;
  indicator->rmt_gpio = rmt_gpio;
  return ESP_OK;
}

esp_err_t dc_indicator_create_rgb(dc_indicator_t* indicator, uint8_t r_gpio, uint8_t g_gpio, uint8_t b_gpio)
{
  indicator->is_rmt = false;
  indicator->r_gpio = r_gpio;
  indicator->g_gpio = g_gpio;
  indicator->b_gpio = b_gpio;
  return ESP_OK;
}

esp_err_t dc_indicator_initialize(dc_indicator_t* indicator)
{
  esp_err_t err = ESP_OK;
  if (indicator->is_rmt)
  {
    // Initialize using RMT
    led_strip_config_t strip_config = {.strip_gpio_num = indicator->rmt_gpio,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812, // Always this one
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {.invert_out = false}};

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT, .resolution_hz = (10 * 1000 * 1000), .mem_block_symbols = 0, .flags = {.with_dma = false}};

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_new_rmt_device(&strip_config, &rmt_config, &indicator->led_strip_handle));

    if (err != ESP_OK)
    {
      indicator->enabled = false;
    }

    return err;
  }
  else
  {
    // Initialize using multiples GPIO
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_direction(indicator->r_gpio, GPIO_MODE_OUTPUT));
    if (err != ESP_OK)
      return err;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_direction(indicator->g_gpio, GPIO_MODE_OUTPUT));
    if (err != ESP_OK)
      return err;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_direction(indicator->b_gpio, GPIO_MODE_OUTPUT));
    if (err != ESP_OK)
      return err;
  }
  return err;
}

esp_err_t dc_indicator_set_color(dc_indicator_t* indicator, uint32_t r, uint32_t g, uint32_t b)
{
  esp_err_t err = ESP_OK;
  if (!indicator->enabled)
  {
    return err;
  }

  if (indicator->is_rmt)
  {
    if (r > CONFIG_DC_INDICATOR_MAX_VALUE)
      r = CONFIG_DC_INDICATOR_MAX_VALUE;

    if (g > CONFIG_DC_INDICATOR_MAX_VALUE)
      g = CONFIG_DC_INDICATOR_MAX_VALUE;

    if (b > CONFIG_DC_INDICATOR_MAX_VALUE)
      b = CONFIG_DC_INDICATOR_MAX_VALUE;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_set_pixel(indicator->led_strip_handle, 0, r, g, b));
    if (err != ESP_OK)
      return err;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_refresh(indicator->led_strip_handle));
    if (err != ESP_OK)
      return err;
  }
  else
  {
    err = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(indicator->r_gpio, r > 0 ? 1 : 0));
    if (err != ESP_OK)
      return err;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(indicator->g_gpio, g > 0 ? 1 : 0));
    if (err != ESP_OK)
      return err;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(gpio_set_level(indicator->b_gpio, b > 0 ? 1 : 0));
    if (err != ESP_OK)
      return err;
  }

  return err;
}

esp_err_t dc_indicator_set_state(dc_indicator_t* indicator, dc_indicator_state_t state)
{
  esp_err_t err = ESP_OK;
  if (!indicator->enabled)
  {
    return err;
  }
  uint32_t r, g, b;
  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_state_to_color(state, &r, &g, &b));
  if (err)
    return err;

  err = ESP_ERROR_CHECK_WITHOUT_ABORT(dc_indicator_set_color(indicator, r, g, b));

  return err;
}

esp_err_t dc_state_to_color(dc_indicator_state_t state, uint32_t* r, uint32_t* g, uint32_t* b)
{
  switch (state)
  {
  case DC_INDICATOR_STATE_NONE:
    *r = 0;
    *g = 0;
    *b = 0;
    break;
  case DC_INDICATOR_STATE_SETUP:
    *r = 0;
    *g = 0;
    *b = 255;
    break;
  case DC_INDICATOR_STATE_SUCCESS:
    *r = 0;
    *g = 255;
    *b = 0;
    break;
  case DC_INDICATOR_STATE_WARNING:
    *r = 255;
    *b = 255;
    *g = 0;
    break;
  case DC_INDICATOR_STATE_ERROR:
    *r = 255;
    *g = 0;
    *b = 0;
    break;
  case DC_INDICATOR_STATE_FATAL:
    *r = 255;
    *g = 255;
    *b = 255;
    break;
  case DC_INDICATOR_STATE_IDLE:
    *r = 0;
    *g = 0;
    *b = 0;
    break;

  default:
    *r = 0;
    *g = 0;
    *b = 0;
    break;
  }

  return ESP_OK;
}