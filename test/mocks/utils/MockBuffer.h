/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/utils/MockBuffer.h
 * DESCRIPTION: GoogleMock implementation for IBuffer interface.
 */

#ifndef MOCK_BUFFER_H
#define MOCK_BUFFER_H

#include <gmock/gmock.h>
#include "utils/IBuffer.h"

namespace Utils {

class MockBuffer : public IBuffer {
public:
    MOCK_METHOD(size_t, write, (const uint8_t* data, size_t len), (override));
    MOCK_METHOD(size_t, read, (uint8_t* dest, size_t len), (override));
    MOCK_METHOD(void, clear, (), (override));
    MOCK_METHOD(size_t, available, (), (const, override));
    MOCK_METHOD(size_t, capacity, (), (const, override));
};

} // namespace Utils

#endif // MOCK_BUFFER_H
