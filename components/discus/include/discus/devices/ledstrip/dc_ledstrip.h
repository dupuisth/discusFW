#ifndef DC_LEDSTRIP_H
#define DC_LEDSTRIP_H

#include <led_strip.h>
#include <sdkconfig.h>
#include <stdbool.h>
#include <stdint.h>
#include <esp_err.h>
#include <discus/status/dc_status_types.h>

typedef struct
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} dc_rgb8_t;

typedef struct
{
  bool enabled;

  uint8_t gpio;
  led_strip_handle_t led_strip_handle;

  uint32_t pixel_count;
} dc_ledstrip_t;

esp_err_t dc_ledstrip_create(uint8_t gpio, uint32_t pixel_count, dc_ledstrip_t* ledstrip);

/// @brief Delete the given ledstrip (delete the ledstrip handle). Also sets enabled to false and pixel_count to 0. The gpio remains unchanged.
esp_err_t dc_ledstrip_delete(dc_ledstrip_t* ledstrip);

esp_err_t dc_ledstrip_set_pixel(dc_ledstrip_t* ledstrip, uint32_t pixel_index, dc_rgb8_t rgb);
esp_err_t dc_ledstrip_fill_pixels(dc_ledstrip_t* ledstrip, dc_rgb8_t rgb);
esp_err_t dc_ledstrip_clear_pixels(dc_ledstrip_t* ledstrip);

esp_err_t dc_ledstrip_flush(dc_ledstrip_t* ledstrip);

#endif // DC_LEDSTRIP_H