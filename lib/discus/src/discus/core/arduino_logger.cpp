// discus/core/logger.cpp
#include <cstdio> // vsnprintf
#include <discus/core/logger.hpp>

#ifdef DISCUS_ARDUINO
#include <Arduino.h>

namespace discus::core::logger
{

LoggerLevel Logger::s_level = LoggerLevel::Info;

void Logger::begin()
{
  Serial.begin(115200);
  delay(300);
}

void Logger::setLevel(LoggerLevel level)
{
  s_level = level;
}

LoggerLevel Logger::getLevel()
{
  return s_level;
}

bool Logger::enabled(LoggerLevel lvl)
{
  if (s_level == LoggerLevel::Off)
    return false;
  return static_cast<uint8_t>(lvl) >= static_cast<uint8_t>(s_level);
}

const char* Logger::prefix(LoggerLevel lvl)
{
  switch (lvl)
  {
  case LoggerLevel::Trace:
    return "[TRACE] ";
  case LoggerLevel::Debug:
    return "[DEBUG] ";
  case LoggerLevel::Info:
    return "[INFO ] ";
  case LoggerLevel::Warn:
    return "[WARN ] ";
  case LoggerLevel::Error:
    return "[ERROR] ";
  case LoggerLevel::Fatal:
    return "[FATAL] ";
  default:
    return "";
  }
}

void Logger::logv(LoggerLevel lvl, const char* fmt, va_list args)
{
  if (!enabled(lvl))
    return;

  Serial.print(prefix(lvl));

  char buf[256]; // adjust size for your target
  vsnprintf(buf, sizeof(buf), fmt, args);
  Serial.print(buf);
}

void Logger::trace(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  logv(LoggerLevel::Trace, fmt, args);
  va_end(args);
}

void Logger::debug(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  logv(LoggerLevel::Debug, fmt, args);
  va_end(args);
}

void Logger::info(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  logv(LoggerLevel::Info, fmt, args);
  va_end(args);
}

void Logger::warn(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  logv(LoggerLevel::Warn, fmt, args);
  va_end(args);
}

void Logger::error(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  logv(LoggerLevel::Error, fmt, args);
  va_end(args);
}

void Logger::fatal(const char* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  logv(LoggerLevel::Fatal, fmt, args);
  va_end(args);
}
} // namespace discus::core
#endif
