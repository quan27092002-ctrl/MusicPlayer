/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/utils/MockThreadSafeQueue.h
 * DESCRIPTION: GoogleMock implementation for IThreadSafeQueue interface.
 * NOTE: This is a templated mock - use MockThreadSafeQueue<T> instantiation.
 */

#ifndef MOCK_THREAD_SAFE_QUEUE_H
#define MOCK_THREAD_SAFE_QUEUE_H

#include <gmock/gmock.h>
#include "utils/IThreadSafeQueue.h"

namespace Utils {

template <typename T>
class MockThreadSafeQueue : public IThreadSafeQueue<T> {
public:
    MOCK_METHOD(void, push, (const T& value), (override));
    MOCK_METHOD(bool, tryPop, (T& value), (override));
    MOCK_METHOD(void, waitAndPop, (T& value), (override));
    MOCK_METHOD(bool, empty, (), (const, override));
};

} // namespace Utils

#endif // MOCK_THREAD_SAFE_QUEUE_H
