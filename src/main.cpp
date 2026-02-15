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

Color colors[] = {Color::Blue(), Color::Green(), Color::Teal(), Color::Purple(), Color::Red()};

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
  for (int i = 0; i < 1000; i++)
  {
    // 15 pixels of the same color (maybe sometimes will not be 15 because we cut off at 1000 iterations but it's ok)
    // Should keep i as a global and remove the for loop, but this code is just for testing so nvm
    Color color = colors[(i / 15) % (sizeof(colors) / sizeof(Color))];
    led_service.propagateFromCenter(color);

    led_service.show();
    led_service.blur(0.2, true);
    led_service.dim(0.02);

    delay(1000 / 24);
  }
}