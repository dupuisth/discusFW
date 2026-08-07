#include <esp_check.h>
#include <esp_err.h>
#include <discus_ambilight/dc_ambilight.h>

static const char* TAG = "dc_ambilight";

esp_err_t dc_ambilight_shared_data_init(dc_ambilight_shared_data_t* shared)
{
  memset(shared, 0, sizeof(dc_ambilight_shared_data_t));

  shared->frame_mutex = xSemaphoreCreateMutex();
  if (shared->frame_mutex == NULL)
  {
    return ESP_ERR_NO_MEM;
  }

  return ESP_OK;
}

esp_err_t dc_ambilight_shared_data_reconfigure(dc_ambilight_shared_data_t* shared, uint32_t pixel_count)
{
  xSemaphoreTake(shared->frame_mutex, portMAX_DELAY);

  if (shared->pending_pixels == NULL)
  {
    shared->pending_pixels = malloc(pixel_count * sizeof(dc_rgb8_t));
  }
  else
  {
    shared->pending_pixels = realloc(shared->pending_pixels, pixel_count * sizeof(dc_rgb8_t));
  }

  if (shared->pending_pixels == NULL)
  {
    shared->pixel_count = 0;
    xSemaphoreGive(shared->frame_mutex);
    ESP_RETURN_ON_ERROR(ESP_ERR_NO_MEM, TAG, "Failed to allocate pixels!");
  }

  shared->pixel_count = pixel_count;
  xSemaphoreGive(shared->frame_mutex);
  xTaskNotify(shared->led_task, DC_AMBILIGHT_NOTIFY_RECONFIGURE, eSetBits);
  return ESP_OK;
}

esp_err_t dc_ambilight_shared_data_publish(dc_ambilight_shared_data_t* shared, const dc_rgb8_t* pixels, uint32_t pixel_count)
{
  xSemaphoreTake(shared->frame_mutex, portMAX_DELAY);

  // Validate allocation
  if (shared->pending_pixels == NULL)
  {
    xSemaphoreGive(shared->frame_mutex);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Trying to publish data but the pixels are not allocated");
  }

  // Validate size
  if (shared->pixel_count != pixel_count)
  {
    xSemaphoreGive(shared->frame_mutex);
    ESP_RETURN_ON_ERROR(ESP_ERR_INVALID_STATE, TAG, "Trying to publish data but the pixels size does not match");
  }

  // Copy pixels
  // Might be able to swap buffers ? This might be more optimized
  // TODO: Look into buffer swapping instead of memcpy
  memcpy(shared->pending_pixels, pixels, sizeof(dc_rgb8_t) * pixel_count);

  xSemaphoreGive(shared->frame_mutex);

  xTaskNotify(shared->led_task, DC_AMBILIGHT_NOTIFY_FRAME, eSetBits);

  return ESP_OK;
}