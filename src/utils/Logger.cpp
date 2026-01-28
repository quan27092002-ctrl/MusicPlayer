/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/Logger.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Thread-safe logging utility - Implementation file.
 */

#include "Logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>

namespace Utils {

// ============================================================================
// PRIVATE METHODS
// ============================================================================

Logger::Logger() : mLevel(LogLevel::INFO) {}

std::string Logger::getTimestamp() const {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

const char* Logger::getLevelName(LogLevel level) const {
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO ";
        case LogLevel::WARNING: return "WARN ";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "?????";
    }
}

// ============================================================================
// PUBLIC METHODS
// ============================================================================

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mMutex);
    mLevel = level;
}

LogLevel Logger::getLevel() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mLevel;
}

void Logger::log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (level < mLevel) {
        return;  // Skip if below current level
    }

    std::ostream& out = (level >= LogLevel::ERROR) ? std::cerr : std::cout;
    out << "[" << getTimestamp() << "] "
        << "[" << getLevelName(level) << "] "
        << message << std::endl;
}

} // namespace Utils
