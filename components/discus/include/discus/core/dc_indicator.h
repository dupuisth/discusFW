#ifndef DC_INDICATOR_H
#define DC_INDICATOR_H

#include <esp_err.h>
#include <led_strip.h>
#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
  bool enabled;
  bool is_rmt;

  uint8_t rmt_gpio;
  led_strip_handle_t led_strip_handle;

  uint8_t r_gpio;
  uint8_t g_gpio;
  uint8_t b_gpio;

} dc_indicator_t;

typedef enum
{
  DC_INDICATOR_STATE_NONE,
  DC_INDICATOR_STATE_SETUP,
  DC_INDICATOR_STATE_SUCCESS,
  DC_INDICATOR_STATE_WARNING,
  DC_INDICATOR_STATE_ERROR,
  DC_INDICATOR_STATE_FATAL,
  DC_INDICATOR_STATE_IDLE
} dc_indicator_state_t;

esp_err_t dc_indicator_create_rmt(dc_indicator_t* indicator, uint8_t rmt_gpio);
esp_err_t dc_indicator_create_rgb(dc_indicator_t* indicator, uint8_t r_gpio, uint8_t g_gpio, uint8_t b_gpio);

esp_err_t dc_indicator_initialize(dc_indicator_t* indicator);

esp_err_t dc_indicator_set_color(dc_indicator_t* indicator, uint32_t r, uint32_t g, uint32_t b);
esp_err_t dc_indicator_set_state(dc_indicator_t* indicator, dc_indicator_state_t state);

esp_err_t dc_state_to_color(dc_indicator_state_t state, uint32_t* r, uint32_t* g, uint32_t* b);

#endif