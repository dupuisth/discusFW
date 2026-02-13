#include <discus/core/logger.hpp>
#include <discus/drivers/led_strip/ledstrip_adafruit.hpp>
#include <discus/math/common.hpp>

using namespace discus;
using namespace discus::core::logger;

// Use kMax for m_currentBrightness and kMin for m_targetBrightness => force a set on the strip
drivers::led_strip::AdafruitNeoPixelLedStrip::AdafruitNeoPixelLedStrip(uint16_t data_pin,
    uint16_t strip_size,
    math::Color* pixels_buffer,
    neoPixelType type) :
    m_data_pin(data_pin),
    m_strip_size(strip_size),
    m_pixels(pixels_buffer),
    m_handle(m_strip_size, m_data_pin, type),
    m_dirty(true),
    m_limit_milli_amp(0),
    m_current_brightness(math::Color::kMax),
    m_target_brightness(math::Color::kMin),
    m_micro_amp_draw_at_full_per_led(kDefaultMicroAmpDrawAtFullPerLed),
    m_micro_amp_draw_passive_per_led(kDefaultMicroAmpDrawPassivePerLed)
{
}

drivers::led_strip::AdafruitNeoPixelLedStrip::~AdafruitNeoPixelLedStrip()
{
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::begin()
{
  m_handle.begin();
  Logger::debug("[AdafruitNeoPixelLedStrip] begin (strip_size: %d, pin: %d)\n", m_strip_size, m_data_pin);
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::clear()
{
  // Set all the pixels to black
  for (uint16_t i = 0; i < m_strip_size; i++)
  {
    m_pixels[i] = math::Color::Black();
  }
  m_dirty = true;

  Logger::debug("[AdafruitNeoPixelLedStrip] Cleared\n");
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::show()
{
  if (m_dirty)
  {
    for (uint16_t i = 0; i < m_strip_size; i++)
    {
      m_handle.setPixelColor(i, packPixel(m_pixels[i]));
    }
    m_dirty = false;
    m_handle.setBrightness(15);
    m_handle.show();
    Logger::debug("[AdafruitNeoPixelLedStrip] Show\n");
  }
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::setPixel(uint16_t index, const math::Color& color)
{
  m_pixels[index] = color.clamped();
  m_dirty = true;
  Logger::debug("[AdafruitNeoPixelLedStrip] setPixel(%u, {%.2f, %.2f, %.2f})\n", index, color.r, color.g, color.b);
}

math::Color drivers::led_strip::AdafruitNeoPixelLedStrip::getPixel(uint16_t index) const
{
  return m_pixels[index];
}

math::Color* drivers::led_strip::AdafruitNeoPixelLedStrip::getPixels()
{
  return m_pixels;
}

const math::Color* drivers::led_strip::AdafruitNeoPixelLedStrip::getPixels() const
{
  return m_pixels;
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::setBrightness(math::ColorComponent brightness)
{
  m_target_brightness = math::clamp(brightness, math::Color::kMin, math::Color::kMax);
  m_dirty = true;
  Logger::debug("[AdafruitNeoPixelLedStrip] setBrightness %f\n", brightness);
}

math::ColorComponent drivers::led_strip::AdafruitNeoPixelLedStrip::getBrightness() const
{
  return m_target_brightness;
}

uint16_t drivers::led_strip::AdafruitNeoPixelLedStrip::getSize() const
{
  return m_strip_size;
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::setPowerBudgetMilliAmp(uint32_t limit_milli_amp)
{
  m_limit_milli_amp = limit_milli_amp;
  Logger::debug("[AdafruitNeoPixelLedStrip] setPowerBudgetMilliAmp %u\n", limit_milli_amp);
}

uint32_t drivers::led_strip::AdafruitNeoPixelLedStrip::getPowerBudgetMilliAmp() const
{
  return m_limit_milli_amp;
}

void drivers::led_strip::AdafruitNeoPixelLedStrip::setDirty()
{
  m_dirty = true;
}

inline uint32_t drivers::led_strip::AdafruitNeoPixelLedStrip::packPixel(const math::Color& color)
{
  const math::Color c = color.clamped();

  // Conversion from internal representaion to external
  constexpr math::ColorComponent kScale = (1.0 / math::Color::kMax) * 255.0;

  uint8_t r = static_cast<uint8_t>(c.r * kScale + 0.5);
  uint8_t g = static_cast<uint8_t>(c.g * kScale + 0.5);
  uint8_t b = static_cast<uint8_t>(c.b * kScale + 0.5);

  return m_handle.Color(r, g, b);
}

math::ColorComponent drivers::led_strip::AdafruitNeoPixelLedStrip::calculateOptimalBrightness() const
{
  double total_budget_ua = static_cast<double>(m_limit_milli_amp * 1000);
  double pixels_budget_ua = total_budget_ua - static_cast<double>(m_micro_amp_draw_passive_per_led * m_strip_size);

  // Consumption at full brightness, using the current pixels (discard passive)
  double pixels_consumption_ua_fb = 0.0;
  for (uint16_t i = 0; i < m_strip_size; i++)
  {
    math::Color pixel = m_pixels[i].clamped();
    pixels_consumption_ua_fb =
        ((pixel.r / math::Color::kMax + pixel.g / math::Color::kMax + pixel.b / math::Color::kMax) / 3.0) * m_micro_amp_draw_at_full_per_led;
  }

  // Consumption at target brightness
  double pixels_consumption_ua_tb = pixels_consumption_ua_fb * (m_target_brightness / math::Color::kMax);
  if (pixels_consumption_ua_tb < pixels_budget_ua)
  {
    return m_target_brightness;
  }

  return math::clamp((pixels_budget_ua / pixels_consumption_ua_fb) * math::Color::kMax, math::Color::kMin, math::Color::kMax);
}