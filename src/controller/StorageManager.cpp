/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/StorageManager.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of StorageManager.
 */

#include "StorageManager.h"
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <pwd.h>

namespace fs = std::filesystem;

namespace Controller {

StorageManager::StorageManager() : mNeedsRefresh(true) {}

void StorageManager::refreshDevices() {
    mNeedsRefresh = true;
    mCachedDevices.clear();
}

std::string StorageManager::getUsername() {
    uid_t uid = geteuid();
    struct passwd *pw = getpwuid(uid);
    if (pw) {
        return std::string(pw->pw_name);
    }
    return "";
}

std::vector<StorageDevice> StorageManager::getAvailableStorage() {
    std::vector<StorageDevice> devices;

    // 1. Internal Storage (Default) - REMOVED as per user request (it is auto-loaded)
    // if (fs::exists("./mMusic") && fs::is_directory("./mMusic")) {
    //     devices.push_back({"Internal Storage (mMusic)", "./mMusic"});
    // }

    // 2. Scan /media/[username]/
    std::string username = getUsername();
    if (!username.empty()) {
        std::string mediaPath = "/media/" + username;
        if (fs::exists(mediaPath) && fs::is_directory(mediaPath)) {
            for (const auto& entry : fs::directory_iterator(mediaPath)) {
                if (entry.is_directory()) {
                    // Only add if it contains music files
                    if (hasMusicFiles(entry.path().string())) {
                        devices.push_back({
                            "USB: " + entry.path().filename().string(),
                            entry.path().string()
                        });
                    }
                }
            }
        }
    }

    // 3. Scan /mnt/ (Optional, varies by OS distro)
    if (fs::exists("/mnt") && fs::is_directory("/mnt")) {
         for (const auto& entry : fs::directory_iterator("/mnt")) {
            if (entry.is_directory()) {
                // Only add if it contains music files
                if (hasMusicFiles(entry.path().string())) {
                    devices.push_back({
                        "External: " + entry.path().filename().string(),
                        entry.path().string()
                    });
                }
            }
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
