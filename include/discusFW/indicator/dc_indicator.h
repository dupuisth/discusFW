#ifndef DC_INDICATOR_H
#define DC_INDICATOR_H

#include "esp_err.h"

#include <discusFW/indicator/dc_indicator_types.h>

/**
 * Initialize the indicator
 */
esp_err_t dc_indicator_initialize(void);

/**
 * Set the indicator state
 */
esp_err_t dc_indicator_set_state(dc_indicator_state_id_t state);

#endif