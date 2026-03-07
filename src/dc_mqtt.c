#include "dc_mqtt.h"

esp_mqtt_client_handle_t mqtt_client = NULL;

typedef struct
{
  bool in_use;
  char topic[DC_MQTT_MAX_TOPIC_LEN];
  dc_mqtt_topic_callback_t callback;
  void* user_ctx;
} dc_mqtt_topic_handler_entry_t;

static dc_mqtt_topic_handler_entry_t s_handlers[DC_MQTT_MAX_TOPIC_HANDLERS] = {0};
static portMUX_TYPE s_handlers_lock = portMUX_INITIALIZER_UNLOCKED;
static bool s_connected = false;
static portMUX_TYPE s_state_lock = portMUX_INITIALIZER_UNLOCKED;

static TaskHandle_t s_heartbeat_task = NULL;

static bool dc_mqtt_is_connected()
{
  bool state;
  taskENTER_CRITICAL(&s_state_lock);
  state = s_connected;
  taskEXIT_CRITICAL(&s_state_lock);

  return state;
}

static esp_err_t dc_mqtt_send_heartbeat(void)
{
  char topic[128];
  char payload[256];

  snprintf(topic, sizeof(topic), "device/%s/heartbeat", dc_device_id);
  snprintf(payload,
      sizeof(payload),
      "{\"id\": \"%s\", \"uptime\": %" PRIu32 ", \"heap\": %" PRIu32 "}",
      dc_device_id,
      (uint32_t)(esp_log_timestamp() / 1000),
      esp_get_free_heap_size());

  return dc_mqtt_publish(topic, payload, strlen(payload), 0, 0, 0);
}

static void dc_mqtt_heartbeat_task(void* arg)
{
  while (1)
  {
    if (dc_mqtt_is_connected())
    {
      dc_mqtt_send_heartbeat();
    }

    // 10s
    vTaskDelay(pdMS_TO_TICKS(10000));
  }
}

static void dc_mqtt_dispatch_message(const char* topic, const uint8_t* data, size_t data_len)
{
  dc_mqtt_topic_handler_entry_t handlers_copy[DC_MQTT_MAX_TOPIC_HANDLERS];

  // Copy all the handlers
  // Other option: keep the lock while iterating and copy only when a match is found to call it
  // But since there can be multiple calls on the same topics, this option might be better
  taskENTER_CRITICAL(&s_handlers_lock);
  memcpy(handlers_copy, s_handlers, sizeof(s_handlers));
  taskEXIT_CRITICAL(&s_handlers_lock);

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

static void dc_resubsribe_all(void)
{
  dc_mqtt_topic_handler_entry_t handlers_copy[DC_MQTT_MAX_TOPIC_HANDLERS];

  taskENTER_CRITICAL(&s_handlers_lock);
  memcpy(handlers_copy, s_handlers, sizeof(s_handlers));
  taskEXIT_CRITICAL(&s_handlers_lock);

  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (!handlers_copy[i].in_use)
    {
      continue;
    }

    int msg_id = esp_mqtt_client_subscribe(mqtt_client, handlers_copy[i].topic, 0);
    if (msg_id < 0)
    {
      ESP_LOGW(TAG, "Failed to subsribe to topic=%s", handlers_copy[i].topic);
    }
    else
    {
      ESP_LOGI(TAG, "Subscribe sent for topic=%s, msg_id=%d", handlers_copy[i].topic, msg_id);
    }
  }
}

void dc_mqtt_event_handler(void* handler_args, esp_event_base_t base, int32_t event_id, void* event_data)
{
  ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
  esp_mqtt_event_handle_t event = event_data;
  switch ((esp_mqtt_event_id_t)event_id)
  {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");

    taskENTER_CRITICAL(&s_state_lock);
    s_connected = true;
    taskEXIT_CRITICAL(&s_state_lock);

    dc_resubsribe_all();
    break;

  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");

    taskENTER_CRITICAL(&s_state_lock);
    s_connected = false;
    taskEXIT_CRITICAL(&s_state_lock);
    break;

  case MQTT_EVENT_SUBSCRIBED:
    if (event->data_len > 0 && event->data != NULL)
    {
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d, return code=0x%02x", event->msg_id, (uint8_t)*event->data);
    }
    else
    {
      ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    }
    break;

  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;

  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");

    char topic_buf[DC_MQTT_MAX_TOPIC_LEN];
    // Truncate if the topic_len is too high
    size_t copy_len = (event->topic_len < sizeof(topic_buf) - 1) ? event->topic_len : sizeof(topic_buf) - 1;

    memcpy(topic_buf, event->topic, copy_len);
    topic_buf[copy_len] = '\0';

    ESP_LOGI(TAG, "TOPIC=%.*s\r\n", event->topic_len, event->topic);
    ESP_LOGI(TAG, "DATA=%.*s\r\n", event->data_len, event->data);

    dc_mqtt_dispatch_message(topic_buf, (const uint8_t*)event->data, event->data_len);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
    {
      ESP_LOGI(TAG, "Last error code reported from esp-tls: 0x%x", event->error_handle->esp_tls_last_esp_err);
      ESP_LOGI(TAG, "Last tls stack error number: 0x%x", event->error_handle->esp_tls_stack_err);
      ESP_LOGI(TAG,
          "Last captured errno : %d (%s)",
          event->error_handle->esp_transport_sock_errno,
          strerror(event->error_handle->esp_transport_sock_errno));
    }
    else if (event->error_handle->error_type == MQTT_ERROR_TYPE_CONNECTION_REFUSED)
    {
      ESP_LOGI(TAG, "Connection refused error: 0x%x", event->error_handle->connect_return_code);
    }
    else
    {
      ESP_LOGW(TAG, "Unknown error type: 0x%x", event->error_handle->error_type);
    }
    break;

  default:
    ESP_LOGI(TAG, "Other event id:%d", event->event_id);
    break;
  }
}

esp_err_t dc_mqtt_start(void)
{
  const esp_mqtt_client_config_t mqtt_cfg = {.broker = {
                                                 .address.uri = CONFIG_BROKER_URI,
                                             }};

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
  if (client == NULL)
  {
    return ESP_FAIL;
  }
  /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
  esp_err_t err = esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, dc_mqtt_event_handler, NULL);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to register mqtt client event err=%s", esp_err_to_name(err));
    return err;
  }

  err = esp_mqtt_client_start(client);
  if (err != ESP_OK)
  {
    ESP_LOGE(TAG, "Failed to start mqtt client err=%s", esp_err_to_name(err));
    return err;
  }

  mqtt_client = client;

  if (s_heartbeat_task == NULL)
  {
    xTaskCreate(dc_mqtt_heartbeat_task, "dc_mqtt_heartbeat", 4096, 0, 5, &s_heartbeat_task);
  }

  return ESP_OK;
}

esp_err_t dc_mqtt_register_topic_handler(const char* topic, dc_mqtt_topic_callback_t callback, void* user_ctx)
{
  if (topic == NULL || callback == NULL)
  {
    ESP_LOGW(TAG, "Failed to register topic handler, invalid argument");
    return ESP_ERR_INVALID_ARG;
  }

  if (strlen(topic) >= DC_MQTT_MAX_TOPIC_LEN)
  {
    ESP_LOGW(TAG, "Failed to register mqtt topic handler, topic length too high");
    return ESP_ERR_INVALID_SIZE;
  }

  taskENTER_CRITICAL(&s_handlers_lock); // CRASHES HERE

  // Prevent duplicate
  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (s_handlers[i].in_use && strcmp(s_handlers[i].topic, topic) == 0 && s_handlers[i].callback == callback)
    {
      taskEXIT_CRITICAL(&s_handlers_lock);
      return ESP_ERR_INVALID_STATE;
    }
  }

  // Insert
  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (!s_handlers[i].in_use)
    {
      s_handlers[i].in_use = true;
      s_handlers[i].callback = callback;
      s_handlers[i].user_ctx = user_ctx;
      strcpy(s_handlers[i].topic, topic);
      taskEXIT_CRITICAL(&s_handlers_lock);

      // Subsribe directly ?
      bool subscribe_directly = dc_mqtt_is_connected();

      if (subscribe_directly)
      {
        int msg_id = esp_mqtt_client_subscribe(mqtt_client, topic, 0);
        if (msg_id < 0)
        {
          ESP_LOGW(TAG, "Failed to subsribe to topic=%s (immediate)", topic);
        }
        else
        {
          ESP_LOGW(TAG, "Subscribe sent for topic=%s, msg_id=%d (immediate)", topic, msg_id);
        }
      }
      else
      {
        ESP_LOGI(TAG, "New subscription pending for topic=%s", topic);
      }

      return ESP_OK;
    }
  }

  // Failed to insert, no slot left
  return ESP_ERR_NO_MEM;
}

esp_err_t dc_mqtt_unregister_topic_handler(const char* topic, dc_mqtt_topic_callback_t callback)
{
  if (topic == NULL || callback == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  taskENTER_CRITICAL(&s_handlers_lock);
  for (int i = 0; i < DC_MQTT_MAX_TOPIC_HANDLERS; i++)
  {
    if (s_handlers[i].in_use && strcmp(s_handlers[i].topic, topic) == 0 && s_handlers[i].callback == callback)
    {
      // Hard reset to 0
      memset(&s_handlers[i], 0, sizeof(s_handlers[i]));
      taskEXIT_CRITICAL(&s_handlers_lock);
      return ESP_OK;
    }
  }
  taskEXIT_CRITICAL(&s_handlers_lock);

  return ESP_ERR_NOT_FOUND;
}

esp_err_t dc_mqtt_publish(const char* topic, const void* data, size_t data_len, int qos, int retain, bool buffered)
{
  if (mqtt_client == NULL || topic == NULL)
  {
    return ESP_ERR_INVALID_ARG;
  }

  int msg_id;
  if (buffered)
  {
    msg_id = esp_mqtt_client_enqueue(mqtt_client, topic, (const char*)data, (int)data_len, qos, retain, true);
  }
  else
  {
    msg_id = esp_mqtt_client_publish(mqtt_client, topic, (const char*)data, (int)data_len, qos, retain);
  }

  if (msg_id < 0)
  {
    ESP_LOGW(TAG, "Failed to send topic=%s", topic);
    return ESP_FAIL;
  }

  ESP_LOGI(TAG, "Sent topic=%s, msg_id=%d", topic, msg_id);
  return ESP_OK;
}
