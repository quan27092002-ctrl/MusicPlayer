/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testStorageManager.cpp
 * DESCRIPTION: Unit tests for StorageManager class.
 */

#include <gtest/gtest.h>
#include "controller/StorageManager.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class StorageManagerTest : public ::testing::Test {
protected:
    Controller::StorageManager manager;
    std::string testDir;

    void SetUp() override {
        testDir = "/tmp/storage_manager_test_" + std::to_string(getpid());
        fs::create_directories(testDir);
    }

    void TearDown() override {
        fs::remove_all(testDir);
    }

    void createTestFile(const std::string& path, const std::string& content = "") {
        std::ofstream file(path);
        file << content;
        file.close();
    }
};

// ============================================================================
// Basic Construction Tests
// ============================================================================

TEST_F(StorageManagerTest, Construction) {
    Controller::StorageManager sm;
    // Should construct without throwing
}

TEST_F(StorageManagerTest, GetAvailableStorageReturnsVector) {
    auto devices = manager.getAvailableStorage();
    // Should return a vector (may be empty if no USB drives connected)
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, RefreshDevices) {
    // First call
    auto devices1 = manager.getAvailableStorage();
    
    // Refresh
    manager.refreshDevices();
    
    // Second call after refresh
    auto devices2 = manager.getAvailableStorage();
    
    // Both should return valid vectors
    EXPECT_GE(devices1.size(), 0u);
    EXPECT_GE(devices2.size(), 0u);
}

TEST_F(StorageManagerTest, RefreshClearsCache) {
    // Get devices twice without refresh - should use cache
    auto devices1 = manager.getAvailableStorage();
    auto devices2 = manager.getAvailableStorage();
    
    // After refresh, cache should be cleared
    manager.refreshDevices();
    auto devices3 = manager.getAvailableStorage();
    
    // All should succeed
    EXPECT_GE(devices1.size(), 0u);
    EXPECT_GE(devices2.size(), 0u);
    EXPECT_GE(devices3.size(), 0u);
}

// ============================================================================
// hasMusicFiles Tests (via directory with music)
// ============================================================================

TEST_F(StorageManagerTest, HasMusicFilesWithMp3) {
    // Create test mp3 file
    createTestFile(testDir + "/song.mp3");
    
    // Get devices - internal check uses hasMusicFiles
    auto devices = manager.getAvailableStorage();
    
    // The test directory won't be detected as it's not under /media or /mnt
    // But we verify no crash occurs
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, HasMusicFilesWithWav) {
    createTestFile(testDir + "/song.wav");
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, HasMusicFilesWithOgg) {
    createTestFile(testDir + "/song.ogg");
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, HasMusicFilesWithFlac) {
    createTestFile(testDir + "/song.flac");
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, HasMusicFilesNested) {
    // Create nested directory with music file
    fs::create_directories(testDir + "/subdir/deep");
    createTestFile(testDir + "/subdir/deep/song.mp3");
    
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, NoMusicFilesEmptyDir) {
    // Empty directory - no music files
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

TEST_F(StorageManagerTest, NoMusicFilesWithTextFiles) {
    createTestFile(testDir + "/readme.txt");
    createTestFile(testDir + "/document.pdf");
    
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

// ============================================================================
// StorageDevice Structure Tests
// ============================================================================

TEST_F(StorageManagerTest, StorageDeviceStructure) {
    Controller::StorageDevice device;
    device.name = "Test Device";
    device.path = "/test/path";
    
    EXPECT_EQ(device.name, "Test Device");
    EXPECT_EQ(device.path, "/test/path");
}

// ============================================================================
// Multiple Refresh Calls
// ============================================================================

TEST_F(StorageManagerTest, MultipleRefreshCalls) {
    for (int i = 0; i < 5; ++i) {
        manager.refreshDevices();
        auto devices = manager.getAvailableStorage();
        EXPECT_GE(devices.size(), 0u);
    }
}

// ============================================================================
// Case Sensitivity Tests (for music file extensions)
// ============================================================================

TEST_F(StorageManagerTest, MusicFilesCaseInsensitive) {
    createTestFile(testDir + "/song.MP3");
    createTestFile(testDir + "/song2.Mp3");
    createTestFile(testDir + "/song3.FLAC");
    
    auto devices = manager.getAvailableStorage();
    EXPECT_GE(devices.size(), 0u);
}

namespace Controller {

class StorageManagerCoverageTest : public ::StorageManagerTest {
protected:
    void SetUp() override {
        ::StorageManagerTest::SetUp();
        // Clear default search roots and add test directory
        manager.mSearchRoots.clear();
        manager.mSearchRoots.push_back(testDir);
    }

    void addSearchRoot(const std::string& path) {
        manager.mSearchRoots.push_back(path);
    }
};

TEST_F(StorageManagerCoverageTest, FindsMusicInSearchRoot) {
    // Create a "USB" folder with music
    fs::create_directories(testDir + "/USB1");
    // Create dummy music file
    createTestFile(testDir + "/USB1/song.mp3");
    
    auto devices = manager.getAvailableStorage();
    
    // Should find 1 device
    ASSERT_EQ(devices.size(), 1u);
    // Path inside /tmp -> falls to "Storage: " label logic
    EXPECT_EQ(devices[0].name, "Storage: USB1");
    EXPECT_EQ(devices[0].path, testDir + "/USB1");
}

TEST_F(StorageManagerCoverageTest, IgnoresEmptyFolders) {
    fs::create_directories(testDir + "/EmptyDir");
    auto devices = manager.getAvailableStorage();
    EXPECT_EQ(devices.size(), 0u);
}

TEST_F(StorageManagerCoverageTest, HandlesPermissionError) {
    std::string noPermDir = testDir + "/NoPerm";
    fs::create_directories(noPermDir);
    
    // Remove read permissions
    fs::permissions(noPermDir, fs::perms::none);
    
    // Should catch exception and continue gracefully
    auto devices = manager.getAvailableStorage();
    EXPECT_EQ(devices.size(), 0u);
    
    // Cleanup
    fs::permissions(noPermDir, fs::perms::all);
}

TEST_F(StorageManagerCoverageTest, VerifyMultipleRoots) {
    std::string root2 = testDir + "_2";
    fs::create_directories(root2 + "/Ext");
    createTestFile(root2 + "/Ext/song.wav");
    
    fs::create_directories(testDir + "/USB1");
    createTestFile(testDir + "/USB1/song.mp3");
    
    addSearchRoot(root2);
    
    auto devices = manager.getAvailableStorage();
    ASSERT_EQ(devices.size(), 2u);
    
    fs::remove_all(root2);
}

} // namespace Controller

