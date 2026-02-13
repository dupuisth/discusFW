#include <Arduino.h>
#include <discus/core/logger.hpp>
#include <discus/drivers/led_strip/ledstrip_adafruit.hpp>

#define LEDSTRIP_DATA 4
#define LEDSTRIP_SIZE 115

using namespace discus;
using namespace discus::core::logger;
using namespace discus::math;
using namespace discus::drivers::led_strip;

Color pixels[LEDSTRIP_SIZE];
AdafruitNeoPixelLedStrip led_strip(LEDSTRIP_DATA, LEDSTRIP_SIZE, pixels);

void setup()
{
  Logger::begin();
  Logger::setLevel(LoggerLevel::Debug);
  Logger::trace("Started Logger\n");

  led_strip.begin();
}

void loop()
{
  Logger::debug("loop\n");
  led_strip.setBrightness(1.0);
  led_strip.clear();
  led_strip.setPixel(0, Color::Blue());
  led_strip.show();

  delay(1000);
}