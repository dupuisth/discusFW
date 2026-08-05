#include <driver/gpio.h>
#include <esp_check.h>
#include <discus/indicator/dc_led_indicator.h>
#include <discus/status/dc_status.h>

///////////////////////////////////////////
// Statics
///////////////////////////////////////////

static const char* TAG = "dc_led_indicator";

static dc_led_indicator_t dc_led_indicators[DC_STATUS_DOMAIN_LAST_ENUM];

static void dc_led_indicator_status_callback(dc_status_domain_t domain, dc_status_entry_t status)
{
  ESP_LOGI(TAG, "Received update!!");
  dc_led_indicator_t* target = &dc_led_indicators[domain];
  if (target->enabled == false)
  {
    ESP_LOGI(TAG, "Disabled!!");

    return;
  }

  esp_err_t err;
  uint8_t r, g, b;
  err = dc_led_indicator_color_from_status(domain, status, &r, &g, &b);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to get color from status");
  }

  err = dc_led_indicator_set_color(target, r, g, b);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to set indicator color");
  }
}

///////////////////////////////////////////
// Implementations
///////////////////////////////////////////

esp_err_t dc_led_indicator_init()
{
  for (uint8_t i = 0; i < DC_STATUS_DOMAIN_LAST_ENUM; i++)
  {
    dc_led_indicators[i].enabled = false;
  }

// System
#ifndef CONFIG_DC_LED_INDICATOR_SYSTEM_TYPE_DISABLED
#if CONFIG_DC_LED_INDICATOR_SYSTEM_TYPE_RGB
  ESP_RETURN_ON_ERROR(dc_led_indicator_create_rgb(&dc_led_indicators[DC_STATUS_DOMAIN_SYSTEM],
                          CONFIG_DC_LED_INDICATOR_SYSTEM_RGB_GPIO_R,
                          CONFIG_DC_LED_INDICATOR_SYSTEM_RGB_GPIO_G,
                          CONFIG_DC_LED_INDICATOR_SYSTEM_RGB_GPIO_B),
      TAG,
      "Failed to create System indicator");
#elif CONFIG_DC_LED_INDICATOR_SYSTEM_TYPE_RMT
  ESP_RETURN_ON_ERROR(dc_led_indicator_create_rmt(&dc_led_indicators[DC_STATUS_DOMAIN_SYSTEM], CONFIG_DC_LED_INDICATOR_SYSTEM_RMT_GPIO),
      TAG,
      "Failed to create System indicator");
#else
#error Undefined behavior
#endif
  ESP_RETURN_ON_ERROR(dc_led_indicator_init_indicator(&dc_led_indicators[DC_STATUS_DOMAIN_SYSTEM]), TAG, "Failed to init System indicator");
#endif

// Zigbee
#ifndef CONFIG_DC_LED_INDICATOR_ZIGBEE_TYPE_DISABLED
#if CONFIG_DC_LED_INDICATOR_ZIGBEE_TYPE_RGB
  ESP_RETURN_ON_ERROR(dc_led_indicator_create_rgb(&dc_led_indicators[DC_STATUS_DOMAIN_ZIGBEE],
                          CONFIG_DC_LED_INDICATOR_ZIGBEE_RGB_GPIO_R,
                          CONFIG_DC_LED_INDICATOR_ZIGBEE_RGB_GPIO_G,
                          CONFIG_DC_LED_INDICATOR_ZIGBEE_RGB_GPIO_B),
      TAG,
      "Failed to create Zigbee indicator");
#elif CONFIG_DC_LED_INDICATOR_ZIGBEE_TYPE_RMT
  ESP_RETURN_ON_ERROR(dc_led_indicator_create_rmt(&dc_led_indicators[DC_STATUS_DOMAIN_ZIGBEE], CONFIG_DC_LED_INDICATOR_ZIGBEE_RMT_GPIO),
      TAG,
      "Failed to create Zigbee indicator");
#else
#error Undefined behavior
#endif
  ESP_RETURN_ON_ERROR(dc_led_indicator_init_indicator(&dc_led_indicators[DC_STATUS_DOMAIN_ZIGBEE]), TAG, "Failed to init Zigbee indicator");
#endif

// UART
#ifndef CONFIG_DC_LED_INDICATOR_UART_TYPE_DISABLED
#if CONFIG_DC_LED_INDICATOR_UART_TYPE_RGB
  ESP_RETURN_ON_ERROR(dc_led_indicator_create_rgb(&dc_led_indicators[DC_STATUS_DOMAIN_UART],
                          CONFIG_DC_LED_INDICATOR_UART_RGB_GPIO_R,
                          CONFIG_DC_LED_INDICATOR_UART_RGB_GPIO_G,
                          CONFIG_DC_LED_INDICATOR_UART_RGB_GPIO_B),
      TAG,
      "Failed to create UART indicator");
#elif CONFIG_DC_LED_INDICATOR_UART_TYPE_RMT
  ESP_RETURN_ON_ERROR(dc_led_indicator_create_rmt(&dc_led_indicators[DC_STATUS_DOMAIN_UART], CONFIG_DC_LED_INDICATOR_UART_RMT_GPIO),
      TAG,
      "Failed to create UART indicator");
#else
#error Undefined behavior
#endif
  ESP_RETURN_ON_ERROR(dc_led_indicator_init_indicator(&dc_led_indicators[DC_STATUS_DOMAIN_UART]), TAG, "Failed to init UART indicator");
#endif

  // Register to the status callback
  ESP_RETURN_ON_ERROR(dc_status_manager_register_callback(dc_led_indicator_status_callback, true),
      TAG,
      "Failed to add dc_led_indicator to the status manager callback");

  return ESP_OK;
}

esp_err_t dc_led_indicator_create_rmt(dc_led_indicator_t* indicator, uint8_t rmt_gpio)
{
  indicator->is_rmt = true;
  indicator->rmt_gpio = rmt_gpio;
  return ESP_OK;
}

esp_err_t dc_led_indicator_create_rgb(dc_led_indicator_t* indicator, uint8_t r_gpio, uint8_t g_gpio, uint8_t b_gpio)
{
  indicator->is_rmt = false;
  indicator->r_gpio = r_gpio;
  indicator->g_gpio = g_gpio;
  indicator->b_gpio = b_gpio;
  return ESP_OK;
}

esp_err_t dc_led_indicator_init_indicator(dc_led_indicator_t* indicator)
{
  esp_err_t err = ESP_OK;
  indicator->enabled = false;
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

    err = led_strip_new_rmt_device(&strip_config, &rmt_config, &indicator->led_strip_handle);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to create rmt device");
  }
  else
  {
    // Initialize using multiples GPIO
    err = gpio_set_direction(indicator->r_gpio, GPIO_MODE_OUTPUT);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set r_gpio to output (%u)", (unsigned int)indicator->r_gpio);

    err = gpio_set_direction(indicator->g_gpio, GPIO_MODE_OUTPUT);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set g_gpio to output (%u)", (unsigned int)indicator->g_gpio);

    err = gpio_set_direction(indicator->b_gpio, GPIO_MODE_OUTPUT);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set b_gpio to output (%u)", (unsigned int)indicator->b_gpio);
  }

  if (err == ESP_OK)
  {
    indicator->enabled = true;
  }

  return err;
}

esp_err_t dc_led_indicator_set_color(dc_led_indicator_t* indicator, uint8_t r, uint8_t g, uint8_t b)
{

  esp_err_t err = ESP_OK;
  if (!indicator->enabled)
  {
    return ESP_ERR_INVALID_STATE;
  }

  if (indicator->is_rmt)
  {
    if (r > CONFIG_DC_LED_INDICATOR_MAX_VALUE)
      r = CONFIG_DC_LED_INDICATOR_MAX_VALUE;

    if (g > CONFIG_DC_LED_INDICATOR_MAX_VALUE)
      g = CONFIG_DC_LED_INDICATOR_MAX_VALUE;

    if (b > CONFIG_DC_LED_INDICATOR_MAX_VALUE)
      b = CONFIG_DC_LED_INDICATOR_MAX_VALUE;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_set_pixel(indicator->led_strip_handle, 0, r, g, b));
    if (err != ESP_OK)
      return err;

    err = ESP_ERROR_CHECK_WITHOUT_ABORT(led_strip_refresh(indicator->led_strip_handle));
    if (err != ESP_OK)
      return err;
  }
  else
  {
    ESP_LOGI(TAG,
        "Setting RGB %d %d %d (%u, %u, %u)",
        indicator->r_gpio,
        indicator->g_gpio,
        indicator->b_gpio,
        (unsigned int)r,
        (unsigned int)g,
        (unsigned int)b);
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

#define Q_APPLY(actual, expected, r, g, b, or, og, ob)                                                                                               \
  if (actual == expected)                                                                                                                            \
  {                                                                                                                                                  \
    *r = or;                                                                                                                                         \
    *g = og;                                                                                                                                         \
    *b = ob;                                                                                                                                         \
    return ESP_OK;                                                                                                                                   \
  }

esp_err_t dc_led_indicator_color_from_status(dc_status_domain_t domain, dc_status_entry_t entry, uint8_t* r, uint8_t* g, uint8_t* b)
{
  if (entry.detail_code != 0)
  {
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_DISCONNECTED, r, g, b, 255, 255, 0); // Yellow
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_JOINING, r, g, b, 16, 255, 255);     // Cyan
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_READING, r, g, b, 0, 0, 255);        // Blue
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_WRITING, r, g, b, 255, 0, 255);      // Magenta
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_CONNECTED, r, g, b, 0, 255, 0);      // Green
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_RECONNECTING, r, g, b, 255, 128, 0); // Orange
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_PAIRING, r, g, b, 128, 0, 255);      // Purple
    Q_APPLY(entry.detail_code, DC_STATUS_CONNECTIVITY_ERROR, r, g, b, 255, 0, 0);          // Red

    ESP_LOGW(TAG, "Expected a custom led color for the given entry code %d", entry.detail_code);
  }

  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_UNKNOWN, r, g, b, 0, 0, 0);
  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_DISABLED, r, g, b, 0, 0, 0);
  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_INIT, r, g, b, 0, 0, 255);
  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_OK, r, g, b, 0, 255, 0);
  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_WARNING, r, g, b, 255, 255, 0);
  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_ERROR, r, g, b, 255, 0, 0);
  Q_APPLY(entry.level, DC_STATUS_LEVEL_SYSTEM_FATAL, r, g, b, 255, 255, 255);

  return ESP_OK;
}
