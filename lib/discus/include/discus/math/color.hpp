#pragma once

namespace discus::math
{

using ColorComponent = double;

struct Color
{
  static constexpr ColorComponent kMin = 0.0;
  static constexpr ColorComponent kMax = 1.0;

  ColorComponent r{kMin};
  ColorComponent g{kMin};
  ColorComponent b{kMin};

  constexpr Color() = default;
  constexpr Color(ColorComponent _r, ColorComponent _g, ColorComponent _b) : r(_r), g(_g), b(_b) {}

  static constexpr Color Black() { return Color(kMin, kMin, kMin); }
  static constexpr Color White() { return Color(kMax, kMax, kMax); }
  static constexpr Color Red() { return Color(kMax, kMin, kMin); }
  static constexpr Color Green() { return Color(kMin, kMax, kMin); }
  static constexpr Color Blue() { return Color(kMin, kMin, kMax); }

  Color operator+(const Color& o) const;
  Color operator-(const Color& o) const;
  Color operator*(ColorComponent o) const;
  Color operator/(ColorComponent o) const;
  void operator+=(const Color& o);
  void operator-=(const Color& o);
  void operator*=(ColorComponent s);
  void operator/=(ColorComponent s);

  /// @brief Return true if the color components are between the expected range [kMin, kMax]
  /// @return True if between the range
  bool isValid() const;

  /// @brief Clamp the color components between the expected range [kMin, kMax]
  /// @return The color clamped
  Color clamped() const;

  /// @brief Clamp the color components between the expected range [kMin, kMax] (in place)
  void clampInPlace();
};
} // namespace discus::math