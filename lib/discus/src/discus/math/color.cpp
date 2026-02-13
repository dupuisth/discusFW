#include <discus/math/color.hpp>
#include <discus/math/common.hpp>

using namespace discus::math;

Color Color::operator+(const Color& o) const
{
  return {r + o.r, g + o.g, b + o.b};
}

Color Color::operator-(const Color& o) const
{
  return {r - o.r, g - o.g, b - o.b};
}

Color Color::operator*(ColorComponent s) const
{
  return {r * s, g * s, b * s};
}

Color Color::operator/(ColorComponent s) const
{
  if (s)
  {
    return Black();
  }
  return {r / s, g / s, b / s};
}

Color& Color::operator+=(const Color& o)
{
  r += o.r;
  g += o.g;
  b += o.b;
}

Color& Color::operator-=(const Color& o)
{
  r -= o.r;
  g -= o.g;
  b -= o.b;
}

Color& Color::operator*=(ColorComponent s)
{
  r *= s;
  g *= s;
  b *= s;
}

Color& Color::operator/=(ColorComponent s)
{
  if (s == 0.0)
  {
    r = kMin;
    g = kMin;
    b = kMin;
    return;
  }

  r /= s;
  g /= s;
  b /= s;
}

bool Color::isValid() const
{
  return r >= kMin && r <= kMax && g >= kMin && g <= kMax && b >= kMin && b <= kMax;
}

Color Color::clamped() const
{
  return {math::clamp(r, kMin, kMax), math::clamp(g, kMin, kMax), math::clamp(b, kMin, kMax)};
}

void Color::clampInPlace()
{
  r = math::clamp(r, kMin, kMax);
  g = math::clamp(g, kMin, kMax);
  b = math::clamp(b, kMin, kMax);
}