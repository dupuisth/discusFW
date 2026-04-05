#ifndef DC_CORE_H
#define DC_CORE_H

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "mqtt_client.h"
#include "nvs_flash.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define TAG "discusFW"
#define DC_DEVICE_ID_LEN 32

extern char dc_device_id[DC_DEVICE_ID_LEN];

void dc_get_device_id(char* out, size_t out_len);
esp_err_t dc_core_init(void);

#endif