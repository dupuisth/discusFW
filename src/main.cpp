#include <Arduino.h>
#include <discus/core/logger.hpp>
#include <discus/drivers/led_strip/ledstrip_adafruit.hpp>
#include <discus/math/common.hpp>

#define LEDSTRIP_DATA 4
#define LEDSTRIP_SIZE 116

using namespace discus;
using namespace discus::core::logger;
using namespace discus::math;
using namespace discus::drivers::led_strip;

Color pixels[LEDSTRIP_SIZE];
AdafruitNeoPixelLedStrip led_strip(LEDSTRIP_DATA, LEDSTRIP_SIZE, pixels);

void setup()
{
  Logger::begin();
  Logger::setLevel(LoggerLevel::Info);
  Logger::trace("Started Logger\n");

  led_strip.begin();
  led_strip.clear();
  led_strip.setBrightness(1.0);
  led_strip.setPowerBudgetMilliAmp(1500);
  led_strip.show();
}

void loop()
{
  for (uint16_t i = 0; i < led_strip.getSize(); i++)
  {
    led_strip.setPixel(i,
        Color(math::lerp(math::Color::kMin, math::Color::kMax, (double)(i) / (double)(LEDSTRIP_SIZE)),
            math::lerp(math::Color::kMin, math::Color::kMax / 16.0, (double)(i) / (double)(LEDSTRIP_SIZE)),
            math::lerp(math::Color::kMin, math::Color::kMax / 8.0, (double)(i) / (double)(LEDSTRIP_SIZE))));
    delayMicroseconds(50000);
    led_strip.show();
  }

  for (int i = led_strip.getSize() - 1; i >= 0; i--)
  {
    led_strip.setPixel(i,
        Color(math::lerp(math::Color::kMin, math::Color::kMin, (double)(i) / (double)(LEDSTRIP_SIZE)),
            math::lerp(math::Color::kMax, math::Color::kMin, (double)(i) / (double)(LEDSTRIP_SIZE)),
            math::lerp(math::Color::kMin, math::Color::kMax, (double)(i) / (double)(LEDSTRIP_SIZE))));
    delayMicroseconds(50000);
    led_strip.show();
  }
  led_strip.show();

  delay(100);
}