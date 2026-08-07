#ifndef DC_AMBILIGHT_LEDSTRIP_H
#define DC_AMBILIGHT_LEDSTRIP_H

#include <discus/devices/ledstrip/dc_ledstrip.h>

// Forward
typedef struct dc_ambilight_shared_data_st dc_ambilight_shared_data_t;

typedef struct
{
  dc_ledstrip_t* ledstrip;
  dc_ambilight_shared_data_t* shared;
} dc_ambilight_ledstrip_task_params_t;

typedef struct
{
  dc_ambilight_shared_data_t* shared;
  dc_ledstrip_t* ledstrip;
  dc_rgb8_t* target_pixels;
  uint32_t pixel_count;
} dc_ambilight_ledstrip_task_data_t;

void dc_ambilight_ledstrip_task(void* params);

esp_err_t dc_ambilight_ledstrip_poll_events(dc_ambilight_ledstrip_task_data_t* data);
esp_err_t dc_ambilight_ledstrip_update_leds(dc_ambilight_ledstrip_task_data_t* data);

esp_err_t dc_ambilight_ledstrip_allocate(dc_ambilight_ledstrip_task_data_t* data, uint32_t pixel_count);

#endif