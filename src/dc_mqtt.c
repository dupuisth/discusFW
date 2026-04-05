#include "dc_mqtt.h"

typedef struct
{
  bool in_use;
  char topic[DC_MQTT_MAX_TOPIC_LEN];
  dc_mqtt_topic_callback_t callback;
  void* user_ctx;
} dc_mqtt_topic_handler_entry_t;

static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static dc_mqtt_topic_handler_entry_t s_handlers[DC_MQTT_MAX_TOPIC_HANDLERS] = {0};

static SemaphoreHandle_t s_handlers_mutex = NULL;
static SemaphoreHandle_t s_state_mutex = NULL;

static bool s_connected = false;
static TaskHandle_t s_heartbeat_task = NULL;

bool dc_mqtt_is_connected(void)
{
  bool connected = false;

  if (s_state_mutex == NULL)
  {
    return false;
  }

  if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
  {
    connected = s_connected;
    xSemaphoreGive(s_state_mutex);
  }

  return connected;
}

static void dc_mqtt_set_connected(bool connected)
{
  if (s_state_mutex == NULL)
  {
    return;
  }

  if (xSemaphoreTake(s_state_mutex, portMAX_DELAY) == pdTRUE)
  {
    s_connected = connected;
    xSemaphoreGive(s_state_mutex);
  }
}

static esp_err_t dc_mqtt_send_heartbeat(void)
{
  char topic[DC_MQTT_HEARTBEAT_TOPIC_LEN];
  char payload[DC_MQTT_HEARTBEAT_PAYLOAD_LEN];

  snprintf(topic, sizeof(topic), "device/%s/heartbeat", dc_device_id);
  snprintf(payload,
      sizeof(payload),
      "{\"id\":\"%s\",\"uptime\":%" PRIu32 ",\"heap\":%" PRIu32 "}",
      dc_device_id,
      (uint32_t)(esp_log_timestamp() / 1000),
      (uint32_t)esp_get_free_heap_size());

  return dc_mqtt_publish(topic, payload, strlen(payload), 0, 0, false);
}

static void dc_mqtt_heartbeat_task(void* arg)
{
  (void)arg;

  while (true)
  {
    if (dc_mqtt_is_connected())
    {
      esp_err_t err = dc_mqtt_send_heartbeat();
      if (err != ESP_OK)
      {
        ESP_LOGW(TAG, "Heartbeat publish failed: %s", esp_err_to_name(err));
      }
    }

    vTaskDelay(pdMS_TO_TICKS(CONFIG_DC_MQTT_HEARTBEAT_INTERVAL_MS));
  }
}

static void dc_mqtt_dispatch_message(const char* topic, const uint8_t* data, size_t data_len)
{
  dc_mqtt_topic_handler_entry_t handlers_copy[DC_MQTT_MAX_TOPIC_HANDLERS] = {0};

  if (s_handlers_mutex == NULL)
  {
    return;
  }

  if (xSemaphoreTake(s_handlers_mutex, portMAX_DELAY) == pdTRUE)
  {
    memcpy(handlers_copy, s_handlers, sizeof(s_handlers));
    xSemaphoreGive(s_handlers_mutex);
  }

  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (!handlers_copy[i].in_use)
    {
      continue;
    }

    if (strcmp(handlers_copy[i].topic, topic) == 0)
    {
      handlers_copy[i].callback(topic, data, data_len, handlers_copy[i].user_ctx);
    }
  }
}

static void dc_mqtt_resubscribe_all(void)
{
  dc_mqtt_topic_handler_entry_t handlers_copy[DC_MQTT_MAX_TOPIC_HANDLERS] = {0};

  if (s_mqtt_client == NULL || s_handlers_mutex == NULL)
  {
    return;
  }

  if (xSemaphoreTake(s_handlers_mutex, portMAX_DELAY) == pdTRUE)
  {
    memcpy(handlers_copy, s_handlers, sizeof(s_handlers));
    xSemaphoreGive(s_handlers_mutex);
  }

  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (!handlers_copy[i].in_use)
    {
      continue;
    }

    int msg_id = esp_mqtt_client_subscribe(s_mqtt_client, handlers_copy[i].topic, 0);
    if (msg_id < 0)
    {
      ESP_LOGW(TAG, "Failed to subscribe to topic=%s", handlers_copy[i].topic);
    }
    else
    {
      ESP_LOGI(TAG, "Subscribe sent for topic=%s, msg_id=%d", handlers_copy[i].topic, msg_id);
    }
  }
}

static void dc_mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
  (void)handler_args;
  (void)base;

  esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT connected");
    dc_mqtt_set_connected(true);
    dc_mqtt_resubscribe_all();
    break;

  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGW(TAG, "MQTT disconnected");
    dc_mqtt_set_connected(false);
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT subscribed, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT unsubscribed, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT published, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_DATA:
  {
    char topic_buf[DC_MQTT_MAX_TOPIC_LEN] = {0};
    size_t copy_len = (event->topic_len < (int)(sizeof(topic_buf) - 1)) ? (size_t)event->topic_len : (sizeof(topic_buf) - 1);

    memcpy(topic_buf, event->topic, copy_len);
    topic_buf[copy_len] = '\0';

    dc_mqtt_dispatch_message(topic_buf, (const uint8_t*)event->data, (size_t)event->data_len);
    break;
  }

  case MQTT_EVENT_ERROR:
    ESP_LOGE(TAG, "MQTT error");
    if (event->error_handle != NULL)
    {
      if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
      {
        ESP_LOGE(TAG, "esp-tls err=0x%x", event->error_handle->esp_tls_last_esp_err);
        ESP_LOGE(TAG, "tls stack err=0x%x", event->error_handle->esp_tls_stack_err);
        ESP_LOGE(TAG, "sock errno=%d (%s)", event->error_handle->esp_transport_sock_errno, strerror(event->error_handle->esp_transport_sock_errno));
      }
      else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
      {
        ESP_LOGE(TAG, "connection refused=0x%x", event->error_handle->connect_return_code);
      }
    }
    break;

  default:
    ESP_LOGD(TAG, "Unhandled MQTT event id=%" PRIi32, event_id);
    break;
  }
}

esp_err_t dc_mqtt_start(void)
{
  if (s_handlers_mutex == NULL)
  {
    s_handlers_mutex = xSemaphoreCreateMutex();
    if (s_handlers_mutex == NULL)
    {
      return ESP_ERR_NO_MEM;
    }
  }

  if (s_state_mutex == NULL)
  {
    s_state_mutex = xSemaphoreCreateMutex();
    if (s_state_mutex == NULL)
    {
      return ESP_ERR_NO_MEM;
    }
  }

  const esp_mqtt_client_config_t mqtt_cfg = {
      .broker.address.uri = CONFIG_DC_MQTT_BROKER_URI,
  };

  s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
  if (s_mqtt_client == NULL)
  {
    return ESP_FAIL;
  }

  esp_err_t err = esp_mqtt_client_register_event(s_mqtt_client, ESP_EVENT_ANY_ID, dc_mqtt_event_handler, NULL);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to register MQTT event handler: %s", esp_err_to_name(err));
    return err;
  }

  err = esp_mqtt_client_start(s_mqtt_client);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to start MQTT client: %s", esp_err_to_name(err));
    return err;
  }

  if (s_heartbeat_task == NULL)
  {
    BaseType_t task_ok = xTaskCreate(dc_mqtt_heartbeat_task, "dc_mqtt_heartbeat", 4096, NULL, 5, &s_heartbeat_task);

    if (task_ok != pdPASS)
    {
      s_heartbeat_task = NULL;
      return ESP_ERR_NO_MEM;
    }
  }

  return ESP_OK;
}

esp_err_t dc_mqtt_register_topic_handler(const char* topic, dc_mqtt_topic_callback_t callback, void* user_ctx)
{
  if (topic == NULL || callback == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  if (strlen(topic) >= DC_MQTT_MAX_TOPIC_LEN)
  {
    return ESP_ERR_INVALID_SIZE;
  }

  if (s_handlers_mutex == NULL)
  {
    return ESP_ERR_INVALID_STATE;
  }

  if (xSemaphoreTake(s_handlers_mutex, portMAX_DELAY) != pdTRUE)
  {
    return ESP_FAIL;
  }

  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (s_handlers[i].in_use && strcmp(s_handlers[i].topic, topic) == 0 && s_handlers[i].callback == callback)
    {
      xSemaphoreGive(s_handlers_mutex);
      return ESP_ERR_INVALID_STATE;
    }
  }

  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (!s_handlers[i].in_use)
    {
      s_handlers[i].in_use = true;
      s_handlers[i].callback = callback;
      s_handlers[i].user_ctx = user_ctx;
      strncpy(s_handlers[i].topic, topic, sizeof(s_handlers[i].topic) - 1);
      s_handlers[i].topic[sizeof(s_handlers[i].topic) - 1] = '\0';

      xSemaphoreGive(s_handlers_mutex);

      if (dc_mqtt_is_connected() && s_mqtt_client != NULL)
      {
        int msg_id = esp_mqtt_client_subscribe(s_mqtt_client, topic, 0);
        if (msg_id < 0)
        {
          ESP_LOGW(TAG, "Failed to subscribe to topic=%s", topic);
        }
        else
        {
          ESP_LOGI(TAG, "Subscribe sent for topic=%s, msg_id=%d", topic, msg_id);
        }
      }

      return ESP_OK;
    }
  }

  xSemaphoreGive(s_handlers_mutex);
  return ESP_ERR_NO_MEM;
}

esp_err_t dc_mqtt_unregister_topic_handler(const char* topic, dc_mqtt_topic_callback_t callback)
{
  if (topic == NULL || callback == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  if (s_handlers_mutex == NULL)
  {
    return ESP_ERR_INVALID_STATE;
  }

  if (xSemaphoreTake(s_handlers_mutex, portMAX_DELAY) != pdTRUE)
  {
    return ESP_FAIL;
  }

  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (s_handlers[i].in_use && strcmp(s_handlers[i].topic, topic) == 0 && s_handlers[i].callback == callback)
    {
      memset(&s_handlers[i], 0, sizeof(s_handlers[i]));
      xSemaphoreGive(s_handlers_mutex);
      return ESP_OK;
    }
  }

  xSemaphoreGive(s_handlers_mutex);
  return ESP_ERR_NOT_FOUND;
}

esp_err_t dc_mqtt_publish(const char* topic, const void* data, size_t data_len, int qos, int retain, bool buffered)
{
  if (s_mqtt_client == NULL || topic == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  int msg_id = -1;

  if (buffered)
  {
    msg_id = esp_mqtt_client_enqueue(s_mqtt_client, topic, (const char*)data, (int)data_len, qos, retain, true);
  }
  else
  {
    msg_id = esp_mqtt_client_publish(s_mqtt_client, topic, (const char*)data, (int)data_len, qos, retain);
  }

  if (msg_id < 0)
  {
    ESP_LOGW(TAG, "Failed to publish topic=%s", topic);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Published topic=%s, msg_id=%d", topic, msg_id);
  return ESP_OK;
}