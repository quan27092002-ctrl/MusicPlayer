/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/Logger.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Thread-safe logging utility with log levels - Header file.
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>
#include <sstream>

namespace Utils {

/**
 * @brief Log levels for filtering messages.
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    NONE = 4  // Disable all logging
};

/**
 * @brief Thread-safe singleton logger.
 * 
 * Usage:
 *   Logger::getInstance().setLevel(LogLevel::DEBUG);
 *   LOG_INFO("Player started");
 *   LOG_ERROR("Failed to open file: " << filename);
 */
class Logger {
private:
    LogLevel mLevel;
    mutable std::mutex mMutex;

    // Private constructor for Singleton
    Logger();

    // Get current timestamp as string
    std::string getTimestamp() const;

    // Get log level name
    const char* getLevelName(LogLevel level) const;

public:
    // Delete copy constructor and assignment
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    /**
     * @brief Get singleton instance.
     */
    static Logger& getInstance();

    /**
     * @brief Set minimum log level to display.
     */
    void setLevel(LogLevel level);

    /**
     * @brief Get current log level.
     */
    LogLevel getLevel() const;

    /**
     * @brief Log a message with specified level.
     */
    void log(LogLevel level, const std::string& message);
};

// ============================================================================
// CONVENIENCE MACROS
// ============================================================================

#define LOG_DEBUG(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    Utils::Logger::getInstance().log(Utils::LogLevel::DEBUG, oss.str()); \
} while(0)

#define LOG_INFO(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    Utils::Logger::getInstance().log(Utils::LogLevel::INFO, oss.str()); \
} while(0)

#define LOG_WARNING(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    Utils::Logger::getInstance().log(Utils::LogLevel::WARNING, oss.str()); \
} while(0)

#define LOG_ERROR(msg) do { \
    std::ostringstream oss; \
    oss << msg; \
    Utils::Logger::getInstance().log(Utils::LogLevel::ERROR, oss.str()); \
} while(0)

} // namespace Utils

#endif // LOGGER_H
