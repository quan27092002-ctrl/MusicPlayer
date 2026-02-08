/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/utils/MockLogger.h
 * DESCRIPTION: GoogleMock implementation for ILogger interface.
 */

#ifndef MOCK_LOGGER_H
#define MOCK_LOGGER_H

#include <gmock/gmock.h>
#include "utils/ILogger.h"

namespace Utils {

class MockLogger : public ILogger {
public:
    MOCK_METHOD(void, setLevel, (LogLevel level), (override));
    MOCK_METHOD(LogLevel, getLevel, (), (const, override));
    MOCK_METHOD(void, log, (LogLevel level, const std::string& message), (override));
};

} // namespace Utils

#endif // MOCK_LOGGER_H
