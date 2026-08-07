#include <esp_check.h>
#include <discus/status/dc_status.h>

///////////////////////////////////////////
// Statics
///////////////////////////////////////////
static const char* TAG = "dc_status";

// Will allocate even unused domains
// TODO: Conditionnal to prevent useless memory usage
static dc_status_entry_t dc_status_entries[DC_STATUS_DOMAIN_LAST_ENUM];

#define DC_STATUS_MANAGER_CALLBACK_MAX 8
static dc_status_manager_set_cb_t dc_status_set_callbacks[DC_STATUS_MANAGER_CALLBACK_MAX];
static uint8_t dc_status_callback_idx = 0;

///////////////////////////////////////////
// Implementations
///////////////////////////////////////////

esp_err_t dc_status_manager_set(dc_status_domain_t domain, dc_status_level_t level, int detail_code)
{
  esp_err_t err = ESP_OK;
  if (domain < 0 || domain >= DC_STATUS_DOMAIN_LAST_ENUM)
  {
    err = ESP_ERR_INVALID_ARG;
    ESP_RETURN_ON_ERROR(err, TAG, "Can't set the requested domain, out of range");
  }

  dc_status_entry_t entry = {.level = level, .detail_code = detail_code};
  dc_status_entries[domain] = entry;

  for (uint8_t i = 0; i < dc_status_callback_idx; i++)
  {
    dc_status_set_callbacks[i](domain, entry);
  }

  return err;
}

esp_err_t dc_status_manager_get(dc_status_domain_t domain, dc_status_entry_t* const status)
{
  esp_err_t err = ESP_OK;
  if (domain < 0 || domain >= DC_STATUS_DOMAIN_LAST_ENUM)
  {
    err = ESP_ERR_INVALID_ARG;
    ESP_RETURN_ON_ERROR(err, TAG, "Can't get the requested domain, out of range");
  }

  *status = dc_status_entries[domain];
  return err;
}

esp_err_t dc_status_manager_register_callback(dc_status_manager_set_cb_t callback, int sync_status)
{
  esp_err_t err = ESP_OK;
  if (dc_status_callback_idx >= DC_STATUS_MANAGER_CALLBACK_MAX)
  {
    err = ESP_ERR_INVALID_STATE;
    ESP_RETURN_ON_ERROR(err, TAG, "Can't add more callbacks, increment the callback buffer size");
  }

  dc_status_set_callbacks[dc_status_callback_idx++] = callback;

  if (sync_status)
  {
    err = dc_status_manager_send_status(callback);
    ESP_RETURN_ON_ERROR(err, TAG, "Error while syncing status");
  }

  return err;
}

esp_err_t dc_status_manager_broadcast_status()
{
  for (uint8_t i = 0; i < dc_status_callback_idx; i++)
  {
    for (uint8_t j = 0; j < DC_STATUS_DOMAIN_LAST_ENUM; j++)
    {
      dc_status_set_callbacks[i](j, dc_status_entries[j]);
    }
  }
  return ESP_OK;
}

esp_err_t dc_status_manager_send_status(dc_status_manager_set_cb_t callback)
{
  for (uint8_t j = 0; j < DC_STATUS_DOMAIN_LAST_ENUM; j++)
  {
    callback(j, dc_status_entries[j]);
  }
  return ESP_OK;
}