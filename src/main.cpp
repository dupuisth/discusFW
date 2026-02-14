#include <Arduino.h>
#include <discus/core/logger.hpp>
#include <discus/drivers/led_strip/ledstrip_adafruit.hpp>
#include <discus/math/common.hpp>
#include <discus/services/led_service.hpp>

#define LEDSTRIP_DATA 4
#define LEDSTRIP_SIZE 116

using namespace discus;
using namespace discus::core::logger;
using namespace discus::math;
using namespace discus::drivers::led_strip;
using namespace discus::services;

// This code is just for testing

Color pixels[LEDSTRIP_SIZE];
AdafruitNeoPixelLedStrip led_strip(LEDSTRIP_DATA, LEDSTRIP_SIZE, pixels);
LedService led_service(&led_strip);

void setup()
{
  Logger::begin();
  Logger::setLevel(LoggerLevel::Info);
  Logger::trace("Started Logger\n");

  led_strip.begin();
  led_strip.clear();
  led_strip.setBrightness(1.0);
  led_strip.setPowerBudgetMilliAmp(0);
  led_strip.show();
}

void loop()
{
  for (int i = 0; i < 115; i++)
  {
    if (i % 6 == 0)
    {
      led_service.propagateFromCenter(Color::Teal());
    }
    else if (i % 6 == 1)
    {
      led_service.propagateFromCenter(Color::Purple());
    }
    else if (i % 6 == 2)
    {
      led_service.propagateFromCenter(Color::Red());
    }
    else if (i % 6 == 3)
    {
      led_service.propagateFromCenter(Color::Green());
    }
    else if (i % 6 == 4)
    {
      led_service.propagateFromCenter(Color::Red());
    }
    else if (i % 6 == 5)
    {
      led_service.propagateFromCenter(Color::Green());
    }

    led_service.show();
    led_service.blur(0.1, true);
    led_service.dim(0.05);
    delay(50);
  }
}