#ifndef DC_CORE_H
#define DC_CORE_H

#include <discus/core/dc_indicator.h>
#include <esp_err.h>

extern dc_indicator_t dc_indicator_global;
extern dc_indicator_t dc_indicator_connectivity;

esp_err_t dc_init(void);

#endif
