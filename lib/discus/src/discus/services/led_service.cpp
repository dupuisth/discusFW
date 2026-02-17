#include <discus/services/led_service.hpp>

using namespace discus;
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

math::Color LedService::getPixel(uint16_t index)
{
  return m_driver->getPixel(index);
}

void LedService::setPixel(uint16_t index, const math::Color& color)
{
  m_driver->setPixel(index, color);
}

void LedService::fill(const math::Color& color)
{
  const uint16_t size = m_driver->getSize();
  math::Color* pixels = m_driver->getPixels();
  const math::Color clamped = color.clamped();
  for (uint16_t i = 0; i < size; i++)
  {
    pixels[i] = clamped;
  }
  m_driver->setDirty();
}

void LedService::addPixel(uint16_t index, const math::Color& color)
{
  math::Color final_color = m_driver->getPixel(index);
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
    pixels[i].clampInPlace();
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
  const uint16_t size = m_driver->getSize();
  math::Color* pixels = m_driver->getPixels();
  math::Color calculated[size];

  // Neighbor weight
  const math::ColorComponent wN = ratio * 0.5;

  for (uint16_t i = 0; i < size; i++)
  {
    calculated[i] = math::Color::Black();

    math::ColorComponent used_neighbor_weight = 0;

    // backward neighbor
    if (i > 0)
    {
      calculated[i] += pixels[i - 1] * wN;
      used_neighbor_weight += wN;
    }
    else if (loop && size > 1)
    {
      calculated[i] += pixels[size - 1] * wN;
      used_neighbor_weight += wN;
    }

    // forward neighbor
    if (i + 1 < size)
    {
      calculated[i] += pixels[i + 1] * wN;
      used_neighbor_weight += wN;
    }
    else if (loop && size > 1)
    {
      calculated[i] += pixels[0] * wN;
      used_neighbor_weight += wN;
    }

    const math::ColorComponent wC = (math::ColorComponent)(1 - used_neighbor_weight);
    calculated[i] += pixels[i] * wC;
  }

  for (uint16_t i = 0; i < size; i++)
  {
    pixels[i] = calculated[i].clamped();
  }
  m_driver->setDirty();
}

void LedService::shiftForward(bool loop)
{
  const uint16_t size = m_driver->getSize();
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
  const uint16_t size = m_driver->getSize();
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
  const uint16_t size = m_driver->getSize();
  math::Color* pixels = m_driver->getPixels();

  if (size % 2 == 1)
  {
    int middle = size / 2;
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
    int lowerMiddle = size / 2 - 1;
    int upperMiddle = size / 2;
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

drivers::led_strip::ILedStrip* LedService::getDriver()
{
  return m_driver;
}