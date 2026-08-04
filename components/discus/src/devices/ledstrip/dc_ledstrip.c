#include <esp_check.h>
#include <esp_err.h>
#include <discus/devices/ledstrip/dc_ledstrip.h>

static const char* TAG = "dc_ledstrip";

esp_err_t dc_ledstrip_create(uint8_t gpio, uint32_t pixel_count, dc_ledstrip_t* ledstrip)
{
  ledstrip->gpio = gpio;
  ledstrip->pixel_count = pixel_count;

  // Initialize using RMT
  led_strip_config_t strip_config = {.strip_gpio_num = gpio,
      .max_leds = pixel_count,
      .led_model = LED_MODEL_WS2812, // Always this one
      .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
      .flags = {.invert_out = false}};

  led_strip_rmt_config_t rmt_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT, .resolution_hz = (10 * 1000 * 1000), .mem_block_symbols = 0, .flags = {.with_dma = false}};

  esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &ledstrip->led_strip_handle);
  ESP_RETURN_ON_ERROR(err, TAG, "Failed to create ledstrip device");

  ledstrip->enabled = true;
  return err;
}

esp_err_t dc_ledstrip_set_pixel(dc_ledstrip_t* ledstrip, uint32_t pixel_index, dc_rgb8_t rgb)
{
  esp_err_t err = led_strip_set_pixel(ledstrip->led_strip_handle, pixel_index, rgb.r, rgb.g, rgb.b);
  return err;
}

esp_err_t dc_ledstrip_fill_pixels(dc_ledstrip_t* ledstrip, dc_rgb8_t rgb)
{
  esp_err_t err = ESP_OK;

  for (uint32_t i = 0; i < ledstrip->pixel_count; i++)
  {
    err = led_strip_set_pixel(ledstrip->led_strip_handle, i, rgb.r, rgb.g, rgb.b);
    if (err != ESP_OK)
    {
      return err;
    }
  }
  return err;
}

esp_err_t dc_ledstrip_flush(dc_ledstrip_t* ledstrip)
{
  esp_err_t err = led_strip_refresh(ledstrip->led_strip_handle);
  return err;
}