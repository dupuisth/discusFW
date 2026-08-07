#include <esp_check.h>
#include <esp_err.h>
#include <discus/status/dc_status.h>
#include <discus_ambilight/dc_ambilight.h>
#include <discus_ambilight/ledstrip/dc_ambilight_ledstrip.h>

#define EVENT_POLL_TICK pdMS_TO_TICKS(5)
// #define EVENT_POLL_TICK portMAX_DELAY

static const char* TAG = "dc_ambilight_ledstrip";

void dc_ambilight_ledstrip_task(void* params)
{
  dc_ambilight_ledstrip_task_params_t* casted_params = (dc_ambilight_ledstrip_task_params_t*)params;

  dc_ambilight_ledstrip_task_data_t data;
  memset(&data, 0, sizeof(data));
  data.shared = casted_params->shared;
  data.ledstrip = casted_params->ledstrip;

  TickType_t next_frame = xTaskGetTickCount();

  while (true)
  {
    // First, try to read the events
    if (dc_ambilight_ledstrip_poll_events(&data) != ESP_OK)
    {
      dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
    }

    // Then, update the leds
    if (dc_ambilight_ledstrip_update_leds(&data) != ESP_OK)
    {
      dc_status_manager_set(DC_STATUS_DOMAIN_SYSTEM, DC_STATUS_LEVEL_SYSTEM_WARNING, 0);
    }

    vTaskDelayUntil(&next_frame, pdMS_TO_TICKS(42));
  }
}

esp_err_t dc_ambilight_ledstrip_poll_events(dc_ambilight_ledstrip_task_data_t* data)
{
  uint32_t events;
  xTaskNotifyWait(0, UINT32_MAX, &events, 0);

  if (events & DC_AMBILIGHT_NOTIFY_RECONFIGURE)
  {
    uint32_t new_pixel_count;
    xSemaphoreTake(data->shared->frame_mutex, portMAX_DELAY);
    new_pixel_count = data->shared->pixel_count;
    xSemaphoreGive(data->shared->frame_mutex);

    // Check if the new size is OK
    if (new_pixel_count == 0)
    {
      // Something went wrong (or just deconfigured ?)
      // Clean up
      if (data->target_pixels != NULL)
      {
        free(data->target_pixels);
        data->target_pixels = NULL;
        data->pixel_count = 0;
      }
    }
    else if (data->pixel_count != new_pixel_count)
    {
      esp_err_t err = dc_ambilight_ledstrip_allocate(data, new_pixel_count);
      ESP_RETURN_ON_ERROR(err, TAG, "Failed to reallocate");
    }
  }

  if (events & DC_AMBILIGHT_NOTIFY_FRAME)
  {
    // Validate the size and copy the pending buffer
    xSemaphoreTake(data->shared->frame_mutex, portMAX_DELAY);

    if (data->shared->pixel_count != data->pixel_count)
    {
      xSemaphoreGive(data->shared->frame_mutex);
      ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Won't update the frame, pixel count mismatch");
    }

    // Copy the buffer
    memcpy(data->target_pixels, data->shared->pending_pixels, sizeof(dc_rgb8_t) * data->pixel_count);

    xSemaphoreGive(data->shared->frame_mutex);
  }

  return ESP_OK;
}

esp_err_t dc_ambilight_ledstrip_update_leds(dc_ambilight_ledstrip_task_data_t* data)
{

  if (data->pixel_count == 0 || data->target_pixels == NULL)
  {
    return ESP_OK;
  }

  for (uint32_t i = 0; i < data->pixel_count; i++)
  {
    esp_err_t err = dc_ledstrip_set_pixel(data->ledstrip, i, data->target_pixels[i]);
    ESP_RETURN_ON_ERROR(err, TAG, "Failed to set pixel %d", i);
  }
  esp_err_t err = dc_ledstrip_flush(data->ledstrip);

  return err;
}

esp_err_t dc_ambilight_ledstrip_allocate(dc_ambilight_ledstrip_task_data_t* data, uint32_t pixel_count)
{
  if (data->pixel_count == pixel_count)
  {
    return ESP_OK;
  }

  if (data->target_pixels != NULL)
  {
    data->target_pixels = realloc(data->target_pixels, sizeof(dc_rgb8_t) * pixel_count);
  }
  else
  {
    data->target_pixels = malloc(sizeof(dc_rgb8_t) * pixel_count);
  }

  if (data->target_pixels == NULL)
  {
    ESP_RETURN_ON_ERROR(ESP_ERR_NO_MEM, TAG, "Failed to allocate target_pixels");
  }
  data->pixel_count = pixel_count;

  if (data->ledstrip->pixel_count != 0)
  {
    dc_ledstrip_delete(data->ledstrip);
  }
  dc_ledstrip_create(data->ledstrip->gpio, pixel_count, data->ledstrip);

  ESP_LOGI(TAG, "Allocated target_pixels buffer (%d)", pixel_count);
  return ESP_OK;
}