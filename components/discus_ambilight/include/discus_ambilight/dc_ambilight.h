#ifndef DC_AMBILIGHT_H
#define DC_AMBILIGHT_H

#include <discus/devices/ledstrip/dc_ledstrip.h>

#define DC_AMBILIGHT_NOTIFY_FRAME BIT(0)
#define DC_AMBILIGHT_NOTIFY_RECONFIGURE BIT(1)

typedef struct dc_ambilight_shared_data_st
{
  dc_rgb8_t* pending_pixels;
  uint32_t pixel_count;

  SemaphoreHandle_t frame_mutex;

  TaskHandle_t led_task;
} dc_ambilight_shared_data_t;

/// @brief Initialize the shared data
esp_err_t dc_ambilight_shared_data_init(dc_ambilight_shared_data_t* shared);

esp_err_t dc_ambilight_shared_data_reconfigure(dc_ambilight_shared_data_t* shared, uint32_t pixel_count);

esp_err_t dc_ambilight_shared_data_publish(dc_ambilight_shared_data_t* shared, const dc_rgb8_t* pixels, uint32_t pixel_count);

#endif