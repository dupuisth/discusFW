#include <Arduino.h>
#include <discus/drivers/led_strip/ledstrip_adafruit.hpp>

#define LEDSTRIP_DATA 7
#define LEDSTRIP_SIZE 115

using namespace discus;
using namespace discus::math;
using namespace discus::drivers::led_strip;

Color pixels[LEDSTRIP_SIZE];
AdafruitNeoPixelLedStrip led_strip(LEDSTRIP_DATA, LEDSTRIP_SIZE, pixels);

void setup()
{
}

void loop()
{
  led_strip.setBrightness(1.0);
  led_strip.clear();
  led_strip.setPixel(0, Color::Blue());
  led_strip.show();
  delay(1000);
}