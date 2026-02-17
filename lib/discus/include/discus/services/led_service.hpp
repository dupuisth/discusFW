#pragma once

#include <discus/drivers/led_strip/iled_strip.hpp>

namespace discus::services
{
class LedService
{
public:
  LedService(drivers::led_strip::ILedStrip* led_strip);
  ~LedService();

  /// @brief Show the current state
  void show();

  /// @brief Set all the pixels to black (this does not call show)
  void clear();

  /// @brief Return the pixel at given index
  math::Color getPixel(uint16_t index);
  /// @brief Set the pixel
  void setPixel(uint16_t index, const math::Color& color);

  /// @brief Set all the pixels to the given color
  void fill(const math::Color& color);

  /// @brief Add the color to the given pixel
  void addPixel(uint16_t index, const math::Color& color);

  /// @brief Mix the given pixel with the given color
  /// @param mix [0.0, 1.0] pixels[i] -> color
  void mixPixel(uint16_t index, const math::Color& color, math::ColorComponent mix);

  /// @brief Dim all the pixels by the given ratio
  /// @param ratio Ratio to multiply each color with (1 - ratio)
  void dim(math::ColorComponent ratio);

  /// @brief Dim all the pixels by the given
  /// @param ratio Ratio to reduce the color, each component (r, g, b) will be reduced by kMax * ratio
  void dimLinear(math::ColorComponent ratio);

  /// @brief Blur the pixels by the given ratio
  /// @param ratio 0.0 nothing changes, 1.0 fully blurred between next, current and previous
  /// @param loop Loop the end and the beginning
  void blur(math::ColorComponent ratio, bool loop = false);

  /// @brief Shift all the pixels forward
  /// @param loop loop the end and the beginning of the strip
  void shiftForward(bool loop = false);

  /// @brief Shift all the pixels backward
  /// @param loop loop the end and the beginning of the strip
  void shiftBackward(bool loop = false);

  /// @brief Propagate a color from the center.
  /// The pixels before the center are shifted left
  /// The pixels after the center are shifted right
  void propagateFromCenter(const math::Color& color);

  /// @brief Return the led strip driver
  drivers::led_strip::ILedStrip* getDriver();

protected:
  drivers::led_strip::ILedStrip* m_driver;
};
} // namespace discus::services