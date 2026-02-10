/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/StorageManager.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of IStorageManager.
 *              Manages storage device detection (Internal, USB).
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "IStorageManager.h"
#include <mutex>
#include <sys/types.h>
#include <pwd.h>

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

protected:
    virtual ::uid_t getSystemUid() const;
    virtual struct ::passwd* getPasswordEntry(::uid_t uid) const;
};

} // namespace Controller

#endif // STORAGE_MANAGER_H

