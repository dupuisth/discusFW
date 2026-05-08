#ifndef DC_ZIGBEE_H
#define DC_ZIGBEE_H

#include <discus_zigbee/dc_zigbee_device.h>
#include <esp_err.h>
#include <esp_zigbee_core.h>

#define DC_ZB_INSTALLCODE_POLICY_ENABLE false
#define DC_ZB_ED_AGING_TIMEOUT ESP_ZB_ED_AGING_TIMEOUT_64MIN
#define DC_ZB_ED_KEEP_ALIVE_MS 3000
#define DC_ZB_PRIMARY_CHANNEL_MASK ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK

#define DC_ZB_ZED_CONFIG()                                                                                                                           \
  {                                                                                                                                                  \
      .esp_zb_role = ESP_ZB_DEVICE_TYPE_ED,                                                                                                          \
      .install_code_policy = DC_ZB_INSTALLCODE_POLICY_ENABLE,                                                                                        \
      .nwk_cfg.zed_cfg =                                                                                                                             \
          {                                                                                                                                          \
              .ed_timeout = DC_ZB_ED_AGING_TIMEOUT,                                                                                                  \
              .keep_alive = DC_ZB_ED_KEEP_ALIVE_MS,                                                                                                  \
          },                                                                                                                                         \
  }

#define DC_ZB_DEFAULT_RADIO_CONFIG()                                                                                                                 \
  {                                                                                                                                                  \
      .radio_mode = ZB_RADIO_MODE_NATIVE,                                                                                                            \
  }

#define DC_ZB_DEFAULT_HOST_CONFIG()                                                                                                                  \
  {                                                                                                                                                  \
      .host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE,                                                                                          \
  }

typedef struct dc_zigbee_config
{
  dc_zigbee_device_t* device;
} dc_zigbee_config_t;

esp_err_t dc_zigbee_init(dc_zigbee_config_t* config);

#endif
