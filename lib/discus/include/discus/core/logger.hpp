// discus/core/logger.hpp
#pragma once

#include <cstdarg>
#include <cstdint>

namespace discus::core::logger
{

enum class LoggerLevel : uint8_t
{
  Trace = 0,
  Debug,
  Info,
  Warn,
  Error,
  Fatal,
  Off
};

class Logger
{
public:
  static void begin();
  static void setLevel(LoggerLevel level);
  static LoggerLevel getLevel();

  static void trace(const char* fmt, ...);
  static void debug(const char* fmt, ...);
  static void info(const char* fmt, ...);
  static void warn(const char* fmt, ...);
  static void error(const char* fmt, ...);
  static void fatal(const char* fmt, ...);

private:
  static void logv(LoggerLevel lvl, const char* fmt, va_list args);
  static const char* prefix(LoggerLevel lvl);
  static bool enabled(LoggerLevel lvl);

  static LoggerLevel s_level;
};

} // namespace discus::core::logger
