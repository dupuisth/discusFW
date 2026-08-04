#ifndef DC_AMBILIGHT_TRANSPORT_H
#define DC_AMBILIGHT_TRANSPORT_H

#include <discus/devices/ledstrip/dc_ledstrip.h>

///////////////////////////////////////////
// Forward decl
///////////////////////////////////////////

typedef struct dc_ambilight_transport_handler dc_ambilight_transport_handler_t;

///////////////////////////////////////////
// Functions
///////////////////////////////////////////

// Bulk, modify all pixels
typedef void (*dc_ambilight_set_pixels)(dc_ambilight_transport_handler_t* transport, dc_rgb8_t* pixels, uint32_t pixel_count);

// Single pixel set
typedef void (*dc_ambilight_set_pixel)(dc_ambilight_transport_handler_t* transport, dc_rgb8_t pixel, uint32_t pixel_index);

///////////////////////////////////////////
// Structs
///////////////////////////////////////////
struct dc_ambilight_transport_handler
{
  dc_ambilight_set_pixels set_pixels_handler;
  dc_ambilight_set_pixel set_single_pixel_handler;

  uint8_t transport;
  void* transport_data;
};

#define DC_AMBILIGHT_TRANSPORT_UART 1

#endif