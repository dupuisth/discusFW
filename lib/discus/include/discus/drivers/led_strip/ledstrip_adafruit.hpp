#pragma once

#ifdef DISCUS_FEAT_ADAFRUIT_NEOPIXEL
#include <Adafruit_NeoPixel.h>
#include <discus/drivers/led_strip/iled_strip.hpp>

namespace discus::drivers::led_strip
{
/// @brief Led Strip drived using Adafruit NeoPixel
class AdafruitNeoPixelLedStrip : public ILedStrip
{
public:
  AdafruitNeoPixelLedStrip(uint16_t data_pin, uint16_t strip_size, math::Color* pixels_buffer, neoPixelType type = NEO_GRB + NEO_KHZ800);
  ~AdafruitNeoPixelLedStrip();

  void begin() override;

  void clear() override;
  void show() override;

  void setPixel(uint16_t index, const math::Color& color) override;

  math::Color getPixel(uint16_t index) const override;
  math::Color* getPixels() override;
  const math::Color* getPixels() const override;

  void setBrightness(math::ColorComponent brightness) override;
  math::ColorComponent getBrightness() const override;

  uint16_t getSize() const override;

  void setPowerBudgetMilliAmp(uint32_t limit_milli_amp) override;
  uint32_t getPowerBudgetMilliAmp() const override;

  void setDirty() override;

  /// @brief Convert the pixel from internal to Adafruit_NeoPixels
  inline uint32_t packPixel(const math::Color& color);

  /// @brief Calculate the optimal brightness, the closest to the target brightness that does not
  /// overdraw current
  math::ColorComponent calculateOptimalBrightness() const;

public:
  static const uint32_t kDefaultMicroAmpDrawAtFullPerLed = 40u * 1000;           // Worst case exprimented
  static const uint32_t kDefaultMicroAmpDrawPassivePerLed = (60u * 1000u) / 150; // Experimented with 150 leds

public:
  Adafruit_NeoPixel m_handle;

  math::Color* m_pixels;
  uint16_t m_data_pin;
  uint16_t m_strip_size;

  math::ColorComponent m_target_brightness;
  math::ColorComponent m_current_brightness;

  uint32_t m_limit_milli_amp;
  uint32_t m_micro_amp_draw_at_full_per_led;
  uint32_t m_micro_amp_draw_passive_per_led;

  bool m_dirty;
};
} // namespace discus::drivers::led_strip

#endif