#ifndef DC_CHECKSUM_H
#define DC_CHECKSUM_H

#include <stddef.h>
#include <stdint.h>

uint16_t dc_crc16_ccitt(const uint8_t* data, size_t length, uint16_t initial, uint16_t polynomial);

#endif
