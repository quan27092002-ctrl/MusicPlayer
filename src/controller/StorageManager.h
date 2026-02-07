/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/StorageManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Manages storage device detection (Internal, USB).
 */

#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <string>
#include <vector>
#include <mutex>

namespace Controller {

struct StorageDevice {
    std::string name; // Human readable (e.g., "Internal Storage", "USB Drive")
    std::string path; // System path (e.g., "./mMusic", "/media/user/USB")
};

class StorageManager {
public:
    StorageManager();
    virtual ~StorageManager() = default;

    /**
     * @brief Scans for available storage devices.
     * Always includes the internal "./mMusic" folder.
     * Checks /media/ and /mnt/ for external drives.
     */
    std::vector<StorageDevice> getAvailableStorage();

private:
    std::string getUsername();
    bool hasMusicFiles(const std::string& path);
};

} // namespace Controller

#endif // STORAGE_MANAGER_H
