/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/StorageManager.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of StorageManager.
 */

#include "StorageManager.h"
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <pwd.h>

namespace fs = std::filesystem;

namespace Controller {

StorageManager::StorageManager() : mNeedsRefresh(true) {
    // Initialize default search roots
    std::string username = getUsername();
    if (!username.empty()) {
        mSearchRoots.push_back("/media/" + username);
    }
    mSearchRoots.push_back("/mnt");
}

void StorageManager::refreshDevices() {
    mNeedsRefresh = true;
    mCachedDevices.clear();
}

std::string StorageManager::getUsername() {
    uid_t uid = getSystemUid();
    struct passwd *pw = getPasswordEntry(uid);
    if (pw) {
        return std::string(pw->pw_name);
    }
    return "";
}

::uid_t StorageManager::getSystemUid() const {
    return geteuid();
}

struct ::passwd* StorageManager::getPasswordEntry(::uid_t uid) const {
    return getpwuid(uid);
}

std::vector<StorageDevice> StorageManager::getAvailableStorage() {
    std::vector<StorageDevice> devices;

    for (const auto& rootPath : mSearchRoots) {
        // Skip if root doesn't exist
        if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
            continue;
        }

        try {
            for (const auto& entry : fs::directory_iterator(rootPath)) {
                if (entry.is_directory()) {
                    // Only add if it contains music files
                    if (hasMusicFiles(entry.path().string())) {
                        std::string label;
                        if (rootPath.find("/media/") == 0) {
                            label = "USB: " + entry.path().filename().string();
                        } else if (rootPath.find("/mnt") == 0) {
                            label = "External: " + entry.path().filename().string();
                        } else {
                            // Fallback/Test label
                            label = "Storage: " + entry.path().filename().string();
                        }
                        
                        devices.push_back({label, entry.path().string()});
                    }
                }
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error scanning root " << rootPath << ": " << e.what() << std::endl;
        }
    }

    return devices;
}

bool StorageManager::hasMusicFiles(const std::string& path) {
    try {
        if (!fs::exists(path) || !fs::is_directory(path)) return false;

        // Recursive scan
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // ToLower
                for (auto& c : ext) c = tolower(c);
                
                if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac") {
                    return true; // Found at least one music file
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        // Permission denied or other error
        std::cerr << "Error scanning " << path << ": " << e.what() << std::endl;
    }
    return false;
}

} // namespace Controller
