/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/StorageManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IStorageManager.
 *              Manages storage device detection (Internal, USB).
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "IStorageManager.h"
#include <mutex>

namespace Controller {

/**
 * @brief Concrete implementation of IStorageManager.
 * 
 * Scans local filesystem for available storage devices.
 */
class StorageManager : public IStorageManager {
public:
    StorageManager();
    ~StorageManager() override = default;

    // IStorageManager interface
    std::vector<StorageDevice> getAvailableStorage() override;
    void refreshDevices() override;

private:
    friend class StorageManagerCoverageTest;

    std::vector<StorageDevice> mCachedDevices;
    std::vector<std::string> mSearchRoots;
    bool mNeedsRefresh;
    
    std::string getUsername();
    bool hasMusicFiles(const std::string& path);
};

} // namespace Controller

#endif // STORAGE_MANAGER_H

