#ifndef DC_LED_INDICATOR_H
#define DC_LED_INDICATOR_H

#include <led_strip.h>
#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>
#include <discus/status/dc_status_types.h>

typedef struct
{
  bool enabled;
  bool is_rmt;

  // If RMT
  uint8_t rmt_gpio;
  led_strip_handle_t led_strip_handle;

  // If GPIO
  uint8_t r_gpio;
  uint8_t g_gpio;
  uint8_t b_gpio;

} dc_led_indicator_t;

/// @brief Initialize the led indicators from the config
esp_err_t dc_led_indicator_init();

esp_err_t dc_led_indicator_create_rmt(dc_led_indicator_t* indicator, uint8_t rmt_gpio);
esp_err_t dc_led_indicator_create_rgb(dc_led_indicator_t* indicator, uint8_t r_gpio, uint8_t g_gpio, uint8_t b_gpio);

esp_err_t dc_led_indicator_init_indicator(dc_led_indicator_t* indicator);

esp_err_t dc_led_indicator_set_color(dc_led_indicator_t* indicator, uint8_t r, uint8_t g, uint8_t b);

esp_err_t dc_led_indicator_color_from_status(dc_status_domain_t domain, dc_status_entry_t entry, uint8_t* r, uint8_t* g, uint8_t* b);
#endif // DC_LED_INDICATOR_H