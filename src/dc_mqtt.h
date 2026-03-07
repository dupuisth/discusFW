#ifndef DC_MQTT_H
#define DC_MQTT_H

#include "dc_core.h"

#define DC_MQTT_MAX_TOPIC_HANDLERS 3
#define DC_MQTT_MAX_TOPIC_LEN 128

typedef void (*dc_mqtt_topic_callback_t)(const char* topic, const uint8_t* data, size_t data_len, void* user_ctx);

extern esp_mqtt_client_handle_t mqtt_client;

esp_err_t dc_mqtt_start(void);

esp_err_t dc_mqtt_register_topic_handler(const char* topic, dc_mqtt_topic_callback_t callback, void* user_ctx);

esp_err_t dc_mqtt_unregister_topic_handler(const char* topic, dc_mqtt_topic_callback_t callback);

esp_err_t dc_mqtt_publish(const char* topic, const void* data, size_t data_len, int qos, int retain, bool buffered);

#endif