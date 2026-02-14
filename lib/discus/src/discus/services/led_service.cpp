#include <discus/services/led_service.hpp>

using namespace discus::services;

LedService::LedService(drivers::led_strip::ILedStrip* led_strip) : m_driver(led_strip)
{
}

LedService::~LedService()
{
}

void LedService::show()
{
  m_driver->show();
}

void LedService::clear()
{
  m_driver->clear();
}

discus::math::Color LedService::getPixel(uint16_t index)
{
  return m_driver->getPixel(index);
}

void LedService::setPixel(uint16_t index, const discus::math::Color& color)
{
  m_driver->setPixel(index, color);
}

void LedService::addPixel(uint16_t index, const discus::math::Color& color)
{
  discus::math::Color final_color = m_driver->getPixel(index);
  final_color += color;
  m_driver->setPixel(index, final_color);
}

void LedService::mixPixel(uint16_t index, const math::Color& color, math::ColorComponent mix)
{
  m_driver->setPixel(index, math::Color::lerp(m_driver->getPixel(index), color, mix));
}

void LedService::dim(math::ColorComponent ratio)
{
  math::Color* pixels = m_driver->getPixels();
  ratio = 1.0 - ratio;
  for (uint16_t i = 0; i < m_driver->getSize(); i++)
  {
    pixels[i] *= ratio;
  }
  m_driver->setDirty();
}

void LedService::dimLinear(math::ColorComponent ratio)
{
  math::Color* pixels = m_driver->getPixels();
  for (uint16_t i = 0; i < m_driver->getSize(); i++)
  {
    pixels[i] = (pixels[i] - math::Color::White() * ratio).clamped();
  }
  m_driver->setDirty();
}

void LedService::blur(math::ColorComponent ratio, bool loop)
{
  uint16_t size = m_driver->getSize();

  math::Color* pixels = m_driver->getPixels();
  math::Color calculated[size];

  math::ColorComponent ratio_compo = ratio * 0.5;
  for (uint16_t i = 0; i < size; i++)
  {
    calculated[i] = math::Color::Black();
    if (i > 0)
    {
      calculated[i] += pixels[i - 1] * ratio_compo;
    }
    else if (loop)
    {
      calculated[i] += pixels[size - 1] * ratio_compo;
    }

    if (i < size - 1)
    {
      calculated[i] += pixels[i + 1] * ratio_compo;
    }
    else if (loop)
    {
      calculated[i] += pixels[0] * ratio_compo;
    }

    calculated[i] += pixels[i] * (1 - ratio);
    calculated[i] = calculated[i];
  }

  for (uint16_t i = 0; i < size; i++)
  {
    pixels[i] = calculated[i].clamped();
  }
  m_driver->setDirty();
}

void LedService::shiftForward(bool loop)
{
  uint16_t size = m_driver->getSize();
  math::Color* pixels = m_driver->getPixels();

  math::Color buffer = pixels[size - 1];
  for (int i = size - 1; i >= 1; i--)
  {
    pixels[i] = pixels[i - 1];
  }
  if (loop)
  {
    pixels[0] = buffer;
  }
  m_driver->setDirty();
}

void LedService::shiftBackward(bool loop)
{
  uint16_t size = m_driver->getSize();
  math::Color* pixels = m_driver->getPixels();

  math::Color buffer = pixels[0];
  for (int i = 0; i < size - 1; i++)
  {
    pixels[i] = pixels[i + 1];
  }
  if (loop)
  {
    pixels[size - 1] = buffer;
  }
  m_driver->setDirty();
}

void LedService::propagateFromCenter(const math::Color& color)
{
  uint16_t size = m_driver->getSize();
  math::Color* pixels = m_driver->getPixels();

  if (size % 2 == 1)
  {
    int middle = (size + 1) / 2;
    for (int i = 0; i < middle; i++)
    {
      pixels[i] = pixels[i + 1];
    }

    for (int i = size - 1; i > middle; i--)
    {
      pixels[i] = pixels[i - 1];
    }
    pixels[middle] = color;
  }
  else
  {
    int lowerMiddle = size / 2;
    int upperMiddle = lowerMiddle + 1;
    for (int i = 0; i < lowerMiddle; i++)
    {
      pixels[i] = pixels[i + 1];
    }

    for (int i = size - 1; i > upperMiddle; i--)
    {
      pixels[i] = pixels[i - 1];
    }
    pixels[lowerMiddle] = color;
    pixels[upperMiddle] = color;
  }

  m_driver->setDirty();
}

discus::drivers::led_strip::ILedStrip* LedService::getDriver()
{
  return m_driver;
}