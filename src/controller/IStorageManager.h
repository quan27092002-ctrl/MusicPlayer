/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/IStorageManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for storage device management.
 *              Follows Dependency Inversion Principle (DIP).
 */

#ifndef ISTORAGE_MANAGER_H
#define ISTORAGE_MANAGER_H

#include <string>
#include <vector>

namespace Controller {

/**
 * @brief Storage device information.
 */
struct StorageDevice {
    std::string name; // Human readable (e.g., "Internal Storage", "USB Drive")
    std::string path; // System path (e.g., "./mMusic", "/media/user/USB")
};

/**
 * @brief Interface for storage device management.
 * 
 * Defines contract for detecting and listing available storage devices.
 * Implementations may scan local folders, USB drives, network shares, etc.
 */
class IStorageManager {
public:
    virtual ~IStorageManager() = default;

    /**
     * @brief Get list of available storage devices.
     * @return Vector of StorageDevice structs
     */
    virtual std::vector<StorageDevice> getAvailableStorage() = 0;

    /**
     * @brief Refresh the list of available devices.
     * Call this when USB devices may have changed.
     */
    virtual void refreshDevices() = 0;
};

} // namespace Controller

#endif // ISTORAGE_MANAGER_H
