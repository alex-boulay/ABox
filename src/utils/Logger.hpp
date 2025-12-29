#ifndef ABOX_LOGGER_HPP
#define ABOX_LOGGER_HPP

#include <cstdio>
#include <functional>
#include <sstream>
#include <string>

namespace ABox {

/**
 * @brief Log severity levels
 */
enum class LogLevel { DEBUG, INFO, WARN, ERROR };

/**
 * @brief Callback signature for log messages
 * @param level Log severity level
 * @param category Log category/filter (e.g., "Vulkan", "PER_FRAME")
 * @param message The formatted log message
 */
using LogCallback = std::function<void(LogLevel, const char *, const char *)>;

/**
 * @brief Set the global log callback
 * @param callback Function to handle log messages (nullptr = default console
 * output)
 */
void setLogCallback(LogCallback callback);

/**
 * @brief Get the current log callback
 */
LogCallback getLogCallback();

/**
 * @brief Set minimum log level (messages below this are ignored)
 */
void setLogLevel(LogLevel minLevel);

/**
 * @brief Get current minimum log level
 */
LogLevel getLogLevel();

/**
 * @brief Enable a category filter
 * @param category Category name to enable (e.g., "PER_FRAME", "VERBOSE",
 * "VULKAN_DETAILED")
 */
void enableCategory(const char *category);

/**
 * @brief Disable a category filter
 */
void disableCategory(const char *category);

/**
 * @brief Check if a category is enabled
 */
bool isCategoryEnabled(const char *category);

/**
 * @brief Clear all category filters (all categories become enabled by default)
 */
void clearCategories();

/**
 * @brief Enable all categories (default state)
 */
void enableAllCategories();

/**
 * @brief Disable all categories except those explicitly enabled
 */
void setFilterMode(bool whitelist);

/**
 * @brief Stream-based logger that accumulates output and calls callback on
 * destruction
 */
class LogStream {
  LogLevel           level;
  const char        *category;
  std::ostringstream buffer;
  bool               enabled;

   public:
  LogStream(LogLevel level, const char *category, bool enabled);
  ~LogStream();

  // Stream operator support
  template <typename T> LogStream &operator<<(const T &value)
  {
    if (enabled) {
      buffer << value;
    }
    return *this;
  }

  // Support for std::endl and other stream manipulators
  LogStream &operator<<(std::ostream &(*manip)(std::ostream &));

  // Explicit flush
  void flush();
};

/**
 * @brief Internal logging function - creates a LogStream
 */
LogStream createLogStream(LogLevel level, const char *category);

} // namespace ABox

// Main logging macro - stream-based
#define ABOX_LOG(category, level) ABox::createLogStream(level, category)

// Category-specific macros
#define ABOX_LOG_VULKAN(level) ABOX_LOG("Vulkan", level)
#define ABOX_LOG_PIPELINE(level) ABOX_LOG("Pipeline", level)
#define ABOX_LOG_SHADER(level) ABOX_LOG("Shader", level)
#define ABOX_LOG_DEVICE(level) ABOX_LOG("Device", level)
#define ABOX_LOG_MEMORY(level) ABOX_LOG("Memory", level)
#define ABOX_LOG_RESOURCE(level) ABOX_LOG("Resource", level)

// Per-frame or other filtered categories
#define ABOX_LOG_PER_FRAME ABOX_LOG("PER_FRAME", ABox::LogLevel::DEBUG)
#define ABOX_LOG_VERBOSE ABOX_LOG("VERBOSE", ABox::LogLevel::DEBUG)

// Convenience macros with level (always compiled, filtered at runtime)
#define LOG_DEBUG(category) ABOX_LOG(category, ABox::LogLevel::DEBUG)
#define LOG_INFO(category) ABOX_LOG(category, ABox::LogLevel::INFO)
#define LOG_WARN(category) ABOX_LOG(category, ABox::LogLevel::WARN)
#define LOG_ERROR(category) ABOX_LOG(category, ABox::LogLevel::ERROR)

#endif // ABOX_LOGGER_HPP
