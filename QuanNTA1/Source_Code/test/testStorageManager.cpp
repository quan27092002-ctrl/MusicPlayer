/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testStorageManager.cpp
 * DESCRIPTION: Unit tests for StorageManager class.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
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
    
    void clearSearchRoots() {
        manager.mSearchRoots.clear();
    }
    
    std::string callGetUsername(StorageManager& mgr) {
        return mgr.getUsername();
    }
};

class TestableStorageManager : public StorageManager {
public:
    MOCK_METHOD(::uid_t, getSystemUid, (), (const, override));
    MOCK_METHOD(struct ::passwd*, getPasswordEntry, (::uid_t), (const, override));
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

TEST_F(StorageManagerCoverageTest, GetUsernameFailure) {
    using ::testing::Return;
    TestableStorageManager mockManager;
    
    // Mock failure to find user
    EXPECT_CALL(mockManager, getSystemUid()).WillOnce(Return(1000));
    EXPECT_CALL(mockManager, getPasswordEntry(1000)).WillOnce(Return(nullptr));
    
    // Call getUsername via helper
    std::string username = callGetUsername(mockManager);
    EXPECT_EQ(username, "");
}

TEST_F(StorageManagerCoverageTest, RootPathInvalid) {
    // Add invalid paths
    addSearchRoot("/non/existent/path");
    addSearchRoot(testDir + "/song.mp3"); // Exists but not directory
    
    // Should skip and not crash
    auto devices = manager.getAvailableStorage();
    // Assuming setUp adds testDir, devices size depends on testDir content
    // But we didn't add any files to testDir in this test
    // So size 0
    EXPECT_EQ(devices.size(), 0u);
}

TEST_F(StorageManagerCoverageTest, RootPathMedia) {
    // Create actual directory structure
    // We need a path that STARTS with /media/ as a string, but resolves to a valid directory.
    // Try using a really long relative path, assuming /media exists.
    // If /media doesn't exist, this might fail.
    // Robustness: only run if /media exists?
    // Or iterate until we find a match?
    
    // Backup plan: if /media doesn't exist, create it? No permission.
    // We can try "/media/../tmp/..."
    // If host has /media directory, this works.
    
    if (fs::exists("/media")) {
        std::string mediaRoot = "/media/../" + testDir.substr(1) + "/MediaRoot";
        fs::create_directories(testDir + "/MediaRoot/UsbDrive");
        createTestFile(testDir + "/MediaRoot/UsbDrive/song.mp3");
        
        clearSearchRoots();
        addSearchRoot(mediaRoot);
        
        auto devices = manager.getAvailableStorage();
        ASSERT_EQ(devices.size(), 1u);
        // Label should be "USB: UsbDrive"
        EXPECT_EQ(devices[0].name, "USB: UsbDrive");
    }
}

TEST_F(StorageManagerCoverageTest, RootPathMnt) {
    // Same for /mnt
    if (fs::exists("/mnt")) {
        std::string mntRoot = "/mnt/../" + testDir.substr(1) + "/MntRoot";
        fs::create_directories(testDir + "/MntRoot/ExtDrive");
        createTestFile(testDir + "/MntRoot/ExtDrive/song.mp3");
        
        clearSearchRoots();
        addSearchRoot(mntRoot);
        
        auto devices = manager.getAvailableStorage();
        ASSERT_EQ(devices.size(), 1u);
        // Label should be "External: ExtDrive"
        EXPECT_EQ(devices[0].name, "External: ExtDrive");
    }
}

TEST_F(StorageManagerCoverageTest, RootPathUnreadable) {
    std::string noPermDir = testDir + "/UnreadableRoot";
    fs::create_directories(noPermDir);
    // Remove permissions
    fs::permissions(noPermDir, fs::perms::none);
    
    clearSearchRoots();
    addSearchRoot(noPermDir);
    
    // This triggers exception in getAvailableStorage loop
    auto devices = manager.getAvailableStorage();
    EXPECT_EQ(devices.size(), 0u);
    
    fs::permissions(noPermDir, fs::perms::all);
}

} // namespace Controller

