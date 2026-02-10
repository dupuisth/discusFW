#pragma once
#include <cstdint>
#include <discus/math/color.hpp>

namespace discus::drivers::led_strip
{
/// @brief Drive a led strip
class ILedStrip
{
public:
  virtual ~ILedStrip() = default;

  /// @brief Set all the pixels to black. This does not call show
  virtual void clear() = 0;
  /// @brief Update the led strip to show the current state
  virtual void show() = 0;

  /// @brief Set the color of a given pixel
  virtual void setPixel(uint16_t index, const math::Color& color) = 0;

  /// @brief Get the color of a given pixel
  virtual math::Color getPixel(uint16_t index) const = 0;
  /// @brief Get all the pixels
  virtual math::Color* getPixels() = 0;
  /// @brief Get all the pixels
  virtual const math::Color* getPixels() const = 0;

  /// @brief Set the brightness (in the range [0.0, 1.0]). The led strip will try to apply the
  /// target brightness but if it will exceed the maximum current, the real brightness will be
  /// lower
  virtual void setBrightness(math::ColorComponent brightness) = 0;
  /// @brief Get the target brightness
  virtual math::ColorComponent getBrightness() const = 0;

  /// @brief Return the number of leds in the led strip
  virtual uint16_t getSize() const = 0;

  /// @brief Set the limit in mA that the led strip should not exceed
  virtual void setPowerBudgetMilliAmp(uint32_t limitMilliAmp) = 0;
  /// @brief Get the limit in mA that the led strip should not exceed
  virtual uint32_t getPowerBudgetMilliAmp() const = 0;
};
} // namespace discus::drivers::led_strip