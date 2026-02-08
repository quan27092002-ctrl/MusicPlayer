/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/controller/MockStorageManager.h
 * DESCRIPTION: GoogleMock implementation for IStorageManager interface.
 */

#ifndef MOCK_STORAGE_MANAGER_H
#define MOCK_STORAGE_MANAGER_H

#include <gmock/gmock.h>
#include "controller/IStorageManager.h"

namespace Controller {

class MockStorageManager : public IStorageManager {
public:
    MOCK_METHOD(std::vector<StorageDevice>, getAvailableStorage, (), (override));
    MOCK_METHOD(void, refreshDevices, (), (override));
};

} // namespace Controller

#endif // MOCK_STORAGE_MANAGER_H
