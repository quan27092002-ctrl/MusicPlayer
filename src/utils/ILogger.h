/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/ILogger.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for Logger - enables mocking in unit tests.
 */

#ifndef ILOGGER_H
#define ILOGGER_H

#include <string>

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
 * @brief Abstract interface for logging functionality.
 * 
 * This interface allows dependency injection of logger implementations,
 * making it possible to mock logging in unit tests.
 */
class ILogger {
public:
    virtual ~ILogger() = default;

    /**
     * @brief Set minimum log level to display.
     * @param level Minimum level for messages to be logged
     */
    virtual void setLevel(LogLevel level) = 0;

    /**
     * @brief Get current log level.
     * @return Current minimum log level
     */
    virtual LogLevel getLevel() const = 0;

    /**
     * @brief Log a message with specified level.
     * @param level Severity level of the message
     * @param message The message to log
     */
    virtual void log(LogLevel level, const std::string& message) = 0;
};

} // namespace Utils

#endif // ILOGGER_H
