#pragma once

namespace discus::math
{
template <typename T> constexpr const T& min(const T& a, const T& b)
{
  return (a < b) ? a : b;
}

template <typename T> constexpr const T& max(const T& a, const T& b)
{
  return (a > b) ? a : b;
}

template <typename T> constexpr T clamp(const T& value, const T& low, const T& high)
{
  return (value < low) ? low : (value > high) ? high : value;
}

template <typename T, typename U> constexpr T lerp(const T& a, const T& b, const U& t)
{
  return a + (b - a) * t;
}
} // namespace discus::math