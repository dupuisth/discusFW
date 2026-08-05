#include <discus/checksum/dc_checksum.h>

// https://gist.github.com/tijnkooijmans/10981093
uint16_t dc_crc16_ccitt(const uint8_t* data, size_t length, uint16_t initial, uint16_t polynomial)
{
  uint16_t crc = initial;

  for (size_t i = 0; i < length; ++i)
  {
    crc ^= (uint16_t)data[i] << 8;

    for (unsigned bit = 0; bit < 8; ++bit)
    {
      if ((crc & 0x8000U) != 0U)
      {
        crc = (uint16_t)((crc << 1) ^ polynomial);
      }
      else
      {
        crc <<= 1;
      }
    }
  }

  return crc;
}