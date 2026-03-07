#ifndef DC_CORE_H
#define DC_CORE_H

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include <string.h>

#define TAG "discusFW"

extern char dc_device_id[32];

void dc_get_device_id(char* out, size_t out_len);
#endif