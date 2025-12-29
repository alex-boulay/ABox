#include "Logger.hpp"
#include <cstring>
#include <iostream>
#include <unordered_set>

namespace ABox {

// Global state
static LogCallback                     g_logCallback = nullptr;
static LogLevel                        g_minLogLevel = LogLevel::DEBUG;
static std::unordered_set<std::string> g_enabledCategories{"PER_FRAME"
}; // PER_FRAME disabled by default
static bool                            g_whitelistMode =
    false; // false = all enabled by default (blacklist mode)

// ANSI color codes for terminal output
namespace Color {
  constexpr const char *RESET  = "\033[0m";
  constexpr const char *RED    = "\033[31m";
  constexpr const char *YELLOW = "\033[33m";
  constexpr const char *BLUE   = "\033[34m";
  constexpr const char *GRAY   = "\033[90m";
} // namespace Color

/**
 * @brief Default console logger with color support
 */
static void defaultLogCallback(
    LogLevel    level,
    const char *category,
    const char *message
)
{
  const char *levelStr;
  const char *color;

  switch (level) {
    case LogLevel::DEBUG:
      levelStr = "DEBUG";
      color    = Color::GRAY;
      break;
    case LogLevel::INFO:
      levelStr = "INFO ";
      color    = Color::BLUE;
      break;
    case LogLevel::WARN:
      levelStr = "WARN ";
      color    = Color::YELLOW;
      break;
    case LogLevel::ERROR:
      levelStr = "ERROR";
      color    = Color::RED;
      break;
    default: levelStr = "?????"; color = Color::RESET;
  }

  printf(
      "%s[%s]%s [%s] %s\n",
      color,
      levelStr,
      Color::RESET,
      category,
      message
  );
  fflush(stdout);
}

void setLogCallback(LogCallback callback) { g_logCallback = callback; }

LogCallback getLogCallback()
{
  return g_logCallback ? g_logCallback : defaultLogCallback;
}

void setLogLevel(LogLevel minLevel) { g_minLogLevel = minLevel; }

LogLevel getLogLevel() { return g_minLogLevel; }

void enableCategory(const char *category)
{
  g_enabledCategories.insert(category);
}

void disableCategory(const char *category)
{
  g_enabledCategories.erase(category);
}

bool isCategoryEnabled(const char *category)
{
  if (g_whitelistMode) {
    // Whitelist mode: only enabled if explicitly added
    return g_enabledCategories.count(category) > 0;
  }
  else {
    // Blacklist mode: enabled unless explicitly disabled
    return g_enabledCategories.count(category) == 0;
  }
}

void clearCategories() { g_enabledCategories.clear(); }

void enableAllCategories()
{
  g_whitelistMode = false;
  g_enabledCategories.clear();
}

void setFilterMode(bool whitelist)
{
  g_whitelistMode = whitelist;
  if (!whitelist) {
    // Switching to blacklist mode - clear disabled list
    g_enabledCategories.clear();
  }
}

// LogStream implementation

LogStream::LogStream(LogLevel level, const char *category, bool enabled)
    : level(level)
    , category(category)
    , enabled(enabled)
{
}

LogStream::~LogStream() { flush(); }

LogStream &LogStream::operator<<(std::ostream &(*manip)(std::ostream &))
{
  if (enabled) {
    buffer << manip;
    // If it's std::endl or std::flush, flush immediately
    if (manip == static_cast<std::ostream &(*)(std::ostream &)>(std::endl) ||
        manip == static_cast<std::ostream &(*)(std::ostream &)>(std::flush)) {
      flush();
    }
  }
  return *this;
}

void LogStream::flush()
{
  if (!enabled) {
    return;
  }

  std::string message = buffer.str();
  if (!message.empty()) {
    LogCallback callback = getLogCallback();
    callback(level, category, message.c_str());
    buffer.str(""); // Clear buffer
    buffer.clear();
  }
}

LogStream createLogStream(LogLevel level, const char *category)
{
  // Check filters
  bool enabled = (level >= g_minLogLevel) && isCategoryEnabled(category);
  return LogStream(level, category, enabled);
}

} // namespace ABox
