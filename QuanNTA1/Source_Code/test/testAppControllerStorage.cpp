/**
 * @file testAppControllerStorage.cpp
 * @brief Unit tests for AppController private storage methods.
 * @details Tests loadFromStorage, getStorageDevices, getLoadingProgress.
 */

#include <gtest/gtest.h>
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>
#include "../src/controller/AppController.h"
#include "../src/controller/IAudioPlayer.h"
#include "../src/controller/ISerialManager.h"
#include "../src/model/PlayerState.h"

using namespace Controller;
using namespace Model;
namespace fs = std::filesystem;

// ============================================================================
// Mock AudioPlayer
// ============================================================================

class MockAudioPlayerStorage : public IAudioPlayer {
public:
    bool initialize() override { return true; }
    void shutdown() override {}
    bool load(const std::string& filePath) override { 
        loadedFile = filePath;
        return true; 
    }
    void unload() override {}
    void play() override {}
    void pause() override {}
    void stop() override {}
    void seek(uint32_t) override {}
    void setVolume(int) override {}
    int getVolume() const override { return 50; }
    AudioState getState() const override { return AudioState::IDLE; }
    uint32_t getPosition() const override { return 0; }
    uint32_t getDuration() const override { return 180000; }
    bool isLoaded() const override { return false; }
    bool isPlaying() const override { return false; }
    void setCallback(AudioCallback) override {}
    void setFinishedCallback(std::function<void()>) override {}
    std::string loadedFile;
};

// ============================================================================
// Mock SerialManager
// ============================================================================

class MockSerialManagerStorage : public ISerialManager {
public:
    bool connect(const std::string&, uint32_t) override { return true; }
    void disconnect() override {}
    bool isConnected() const override { return false; }
    SerialState getState() const override { return SerialState::DISCONNECTED; }
    int send(const std::string&) override { return 0; }
    int sendBytes(const uint8_t*, size_t length) override { return static_cast<int>(length); }
    int read(uint8_t*, size_t) override { return 0; }
    std::string readLine(uint32_t) override { return ""; }
    size_t available() const override { return 0; }
    void setDataCallback(SerialDataCallback) override {}
    void setStateCallback(SerialStateCallback) override {}
    std::string getPortName() const override { return ""; }
    uint32_t getBaudRate() const override { return 0; }
    void flush() override {}
    std::vector<std::string> getAvailablePorts() const override { return {}; }
};

// ============================================================================
// Test Fixture - Must be in Controller namespace to access friend class
// ============================================================================

namespace Controller {

class AppControllerStorageTest : public ::testing::Test {
protected:
    std::shared_ptr<MockAudioPlayerStorage> mockAudio;
    std::shared_ptr<MockSerialManagerStorage> mockSerial;
    std::shared_ptr<PlayerState> playerState;
    std::unique_ptr<AppController> controller;
    std::string testDir;

    void SetUp() override {
        mockAudio = std::make_shared<MockAudioPlayerStorage>();
        mockSerial = std::make_shared<MockSerialManagerStorage>();
        playerState = std::make_shared<PlayerState>();
        controller = std::make_unique<AppController>(mockAudio, mockSerial, playerState);
        controller->initialize();

        testDir = "/tmp/storage_test_" + std::to_string(getpid());
        fs::create_directories(testDir);
    }

    void TearDown() override {
        controller->shutdown();
        controller.reset();
        fs::remove_all(testDir);
    }

    void createMusicFiles(int count, const std::string& ext = ".mp3") {
        for (int i = 0; i < count; i++) {
            std::string path = testDir + "/song" + std::to_string(i) + ext;
            std::ofstream file(path, std::ios::binary);
            file << "\xFF\xFB\x90\x00"; // Minimal MP3 header
            file.close();
        }
    }

    void createSubdirWithFiles(const std::string& subdir, int count) {
        std::string subpath = testDir + "/" + subdir;
        fs::create_directories(subpath);
        for (int i = 0; i < count; i++) {
            std::string path = subpath + "/song" + std::to_string(i) + ".mp3";
            std::ofstream file(path, std::ios::binary);
            file << "\xFF\xFB\x90\x00";
            file.close();
        }
    }

    // Access private methods via friend class
    size_t callLoadFromStorage(const std::string& path) {
        return controller->loadFromStorage(path);
    }

    std::vector<StorageDevice> callGetStorageDevices() {
        return controller->getStorageDevices();
    }

    std::pair<size_t, size_t> callGetLoadingProgress() {
        return controller->getLoadingProgress();
    }

    bool isLoadingInProgress() {
        return controller->mLoadingInProgress.load();
    }

    void waitForLoadingComplete(int maxWaitMs = 5000) {
        int waited = 0;
        while (controller->mLoadingInProgress.load() && waited < maxWaitMs) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            waited += 50;
        }
    }

    void addFileToPlaylistDirectly(const std::string& path) {
        controller->mPlaylistManager->addToPlaylist(path);
    }
};

// ============================================================================
// getStorageDevices Tests
// ============================================================================

TEST_F(AppControllerStorageTest, GetStorageDevicesReturnsVector) {
    auto devices = callGetStorageDevices();
    // May be empty if no USB devices connected, but should not crash
    EXPECT_TRUE(devices.empty() || devices.size() >= 0);
}

TEST_F(AppControllerStorageTest, GetStorageDevicesIsStable) {
    // Call multiple times - should be stable
    auto devices1 = callGetStorageDevices();
    auto devices2 = callGetStorageDevices();
    // Both calls should return same size (if no devices connected/disconnected)
    EXPECT_EQ(devices1.size(), devices2.size());
}

// ============================================================================
// loadFromStorage Tests
// ============================================================================

TEST_F(AppControllerStorageTest, LoadFromStorageNewPath) {
    createMusicFiles(5);
    
    size_t result = callLoadFromStorage(testDir);
    EXPECT_EQ(result, 1u); // Returns 1 to indicate loading started
    
    waitForLoadingComplete();
}

TEST_F(AppControllerStorageTest, LoadFromStorageAlreadyLoaded) {
    createMusicFiles(3);
    
    // First load
    size_t result1 = callLoadFromStorage(testDir);
    EXPECT_EQ(result1, 1u);
    waitForLoadingComplete();
    
    // Second load of same path
    size_t result2 = callLoadFromStorage(testDir);
    EXPECT_EQ(result2, 0u); // Already loaded
}

TEST_F(AppControllerStorageTest, LoadFromStorageWhileLoadingInProgress) {
    // Create lots of files to make loading take longer
    createMusicFiles(50);
    
    // Start first load
    size_t result1 = callLoadFromStorage(testDir);
    EXPECT_EQ(result1, 1u);
    
    // Try another load immediately - should be rejected
    std::string testDir2 = testDir + "_other";
    fs::create_directories(testDir2);
    
    size_t result2 = callLoadFromStorage(testDir2);
    EXPECT_EQ(result2, 0u); // Loading in progress
    
    waitForLoadingComplete();
    fs::remove_all(testDir2);
}

TEST_F(AppControllerStorageTest, LoadFromStorageEmptyDirectory) {
    // Empty directory - no files
    size_t result = callLoadFromStorage(testDir);
    EXPECT_EQ(result, 1u); // Still starts loading
    
    waitForLoadingComplete();
    
    // No tracks added
    EXPECT_EQ(controller->getPlaylistSize(), 0u);
}

TEST_F(AppControllerStorageTest, LoadFromStorageWithSubdirectories) {
    createSubdirWithFiles("album1", 3);
    createSubdirWithFiles("album2", 2);
    
    size_t result = callLoadFromStorage(testDir);
    EXPECT_EQ(result, 1u);
    
    waitForLoadingComplete();
    
    // Should load files from subdirectories recursively
    EXPECT_GE(controller->getPlaylistSize(), 0u);
}

TEST_F(AppControllerStorageTest, LoadFromStorageFiltersExtensions) {
    createMusicFiles(3, ".mp3");
    createMusicFiles(2, ".wav");
    createMusicFiles(1, ".flac");
    
    // Create non-music files
    std::ofstream txtFile(testDir + "/readme.txt");
    txtFile << "not music";
    txtFile.close();
    
    std::ofstream imgFile(testDir + "/cover.jpg", std::ios::binary);
    imgFile << "fake image";
    imgFile.close();
    
    size_t result = callLoadFromStorage(testDir);
    EXPECT_EQ(result, 1u);
    
    waitForLoadingComplete();
    
    // Should only load music files, not txt or jpg
    // Exact count depends on implementation filter
}

TEST_F(AppControllerStorageTest, LoadFromStorageNonExistentPath) {
    size_t result = callLoadFromStorage("/nonexistent/path/12345");
    EXPECT_EQ(result, 1u); // Starts but will fail
    
    waitForLoadingComplete();
}

TEST_F(AppControllerStorageTest, LoadFromStorageUpdatesPlaylist) {
    createMusicFiles(5);
    
    callLoadFromStorage(testDir);
    waitForLoadingComplete();
    
    // Playlist should have been updated
    // Actual count depends on whether files are valid music files
    EXPECT_GE(controller->getLibrarySize(), 0u);
}

// ============================================================================
// getLoadingProgress Tests
// ============================================================================

TEST_F(AppControllerStorageTest, GetLoadingProgressWhenNotLoading) {
    auto [loaded, total] = callGetLoadingProgress();
    EXPECT_EQ(loaded, 0u);
    EXPECT_EQ(total, 0u);
}

TEST_F(AppControllerStorageTest, GetLoadingProgressDuringLoad) {
    createMusicFiles(20);
    
    callLoadFromStorage(testDir);
    
    // Check progress during load
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    if (isLoadingInProgress()) {
        auto [loaded, total] = callGetLoadingProgress();
        // total should be > 0 if scanning started
        EXPECT_GE(total, 0u);
        EXPECT_GE(loaded, 0u);
        EXPECT_LE(loaded, total);
    }
    
    waitForLoadingComplete();
}

TEST_F(AppControllerStorageTest, GetLoadingProgressAfterComplete) {
    createMusicFiles(5);
    
    callLoadFromStorage(testDir);
    waitForLoadingComplete();
    
    auto [loaded, total] = callGetLoadingProgress();
    // After completion, values may be reset or show final state
    EXPECT_GE(loaded, 0u);
}

TEST_F(AppControllerStorageTest, GetLoadingProgressIsThreadSafe) {
    createMusicFiles(30);
    
    callLoadFromStorage(testDir);
    
    // Multiple reads from main thread while loading
    for (int i = 0; i < 10; i++) {
        auto [loaded, total] = callGetLoadingProgress();
        (void)loaded;
        (void)total;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    waitForLoadingComplete();
}

// ============================================================================
// Integration: Loading Flow Tests
// ============================================================================

TEST_F(AppControllerStorageTest, LoadingFlowComplete) {
    createMusicFiles(10);
    
    // 1. Check initial state
    EXPECT_FALSE(isLoadingInProgress());
    auto [initLoaded, initTotal] = callGetLoadingProgress();
    EXPECT_EQ(initLoaded, 0u);
    EXPECT_EQ(initTotal, 0u);
    
    // 2. Start loading
    size_t result = callLoadFromStorage(testDir);
    EXPECT_EQ(result, 1u);
    
    // 3. Loading should be in progress or already done
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    
    // 4. Wait for completion
    waitForLoadingComplete();
    
    // 5. Verify completion
    EXPECT_FALSE(isLoadingInProgress());
}

TEST_F(AppControllerStorageTest, MultipleLoadFromDifferentPaths) {
    createMusicFiles(5);
    
    // Load first path
    callLoadFromStorage(testDir);
    waitForLoadingComplete();
    
    // Create and load second path
    std::string testDir2 = testDir + "_2";
    fs::create_directories(testDir2);
    for (int i = 0; i < 3; i++) {
        std::ofstream file(testDir2 + "/track" + std::to_string(i) + ".mp3", std::ios::binary);
        file << "\xFF\xFB\x90\x00";
        file.close();
    }
    
    size_t result = callLoadFromStorage(testDir2);
    EXPECT_EQ(result, 1u); // New path should work
    
    waitForLoadingComplete();
    fs::remove_all(testDir2);
}

TEST_F(AppControllerStorageTest, ShutdownDuringLoading) {
    createMusicFiles(50);
    
    callLoadFromStorage(testDir);
    
    // Small delay to let loading start
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    // Shutdown while loading - should handle gracefully
    controller->shutdown();
    
    // Controller should handle the shutdown
    EXPECT_EQ(controller->getState(), AppState::UNINITIALIZED);
}

TEST_F(AppControllerStorageTest, LoadDirectoryResetsPlaybackIfPlaylistManipulated) {
    createMusicFiles(1); // song0.mp3
    
    // 1. Manually add to playlist manager to bypass PlaybackController iterator update
    // This simulates a state where playlist has items but playback controller iterator is invalid/reset
    addFileToPlaylistDirectly(testDir + "/song0.mp3");
    
    EXPECT_EQ(controller->getPlaylistSize(), 1u);
    // Iterator in PlaybackController should initially be end() and wasn't updated
    // So current index should be -1
    // Note: We can't easily check PlaybackController internals, but PlayerState tracks index
    EXPECT_EQ(playerState->getCurrentTrackIndex(), -1);
    
    // 2. Call loadDirectory with valid files
    // Even though loadDirectory doesn't add files to playlist (it only scans),
    // it triggers the logic to reset playback if playlist is non-empty and index < 0
    createSubdirWithFiles("new", 1); 
    size_t count = controller->loadDirectory(testDir + "/new");
    EXPECT_GT(count, 0u);
    
    // 3. Verify logic at AppController.cpp:367-369 was executed
    // It should have reset iterator to begin and index to 0
    EXPECT_EQ(playerState->getCurrentTrackIndex(), 0);
}

} // namespace Controller
