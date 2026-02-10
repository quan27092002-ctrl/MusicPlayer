/**
 * @file testPlaylistManagerCoverage.cpp
 * @brief Coverage tests for PlaylistManagerImpl
 */

#include <gtest/gtest.h>
#include "controller/appcontroller/PlaylistManager.h"
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;
using namespace Controller;

class PlaylistManagerCoverageTest : public ::testing::Test {
protected:
    PlaylistManagerImpl manager;
    std::string testDir;

    void SetUp() override {
        testDir = "/tmp/pm_coverage_" + std::to_string(getpid());
        fs::create_directories(testDir);
    }

    void TearDown() override {
        manager.stopAsyncLoading();
        fs::remove_all(testDir);
    }

    void createDummyFile(const std::string& name) {
        std::ofstream f(testDir + "/" + name);
        f << "dummy content";
        f.close();
    }
    
    // Helper to create valid-looking MP3 (extension)
    void createMusicFile(const std::string& name) {
        createDummyFile(name);
    }
};

// 1. Getter Validation (Index Checks)
TEST_F(PlaylistManagerCoverageTest, LibraryGettersBoundary) {
    // Empty library
    EXPECT_EQ(manager.getLibraryTrackName(0), "");
    EXPECT_EQ(manager.getLibraryTrackPath(0), "");
    EXPECT_EQ(manager.getLibraryTrackArtist(0), "Unknown Artist");
    EXPECT_EQ(manager.getLibraryTrackAlbum(0), "Unknown Album");
    EXPECT_TRUE(manager.getLibraryTrackCoverArt(0).empty());
    
    // Add one file
    createMusicFile("1.mp3");
    manager.acquireMediaFile(testDir + "/1.mp3");
    
    // Valid index
    EXPECT_EQ(manager.getLibraryTrackName(0), "1.mp3");
    
    // Invalid index
    EXPECT_EQ(manager.getLibraryTrackName(1), "");
}

// 2. Error Handling (Directory Scan)
TEST_F(PlaylistManagerCoverageTest, LoadDirectoryException) {
    // Create unreadable directory
    std::string badDir = testDir + "/bad";
    fs::create_directory(badDir);
    fs::permissions(badDir, fs::perms::none);
    
    // Should catch exception and return 0
    EXPECT_EQ(manager.loadDirectory(badDir), 0u);
    
    // Restore permissions to allow cleanup
    fs::permissions(badDir, fs::perms::all);
}

// 3. Async Loading Logic
TEST_F(PlaylistManagerCoverageTest, AsyncLoadingTriggered) {
    // Create 60 files (batch is 50 usually, but let's check code or set small batch?)
    // loadDirectoryAsync signature: (path, batchSize = 50).
    // user snippet said: "Test case currently provides fewer files than batchSize".
    // I can pass batchSize=10 to force async with fewer files.
    // Or create 60 files. 60 files is safer if batch size is hardcoded in header? 
    // Header line 73: `void loadDirectoryAsync(const std::string& directoryPath, size_t batchSize = 50);`
    // So I can override batchSize!
    
    for (int i = 0; i < 20; ++i) {
        createMusicFile("song_" + std::to_string(i) + ".mp3");
    }
    
    bool progressCalled = false;
    size_t lastLoaded = 0;
    
    manager.setLoadProgressCallback([&](size_t loaded, size_t total) {
        progressCalled = true;
        lastLoaded = loaded;
    });
    
    // Set batch size to 5. Total 20.
    // Sync load: 5. Async: 15.
    manager.loadDirectoryAsync(testDir, 5);
    
    // Wait for completion
    int timeout = 0;
    while (manager.isLoading() && timeout < 20) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        timeout++;
    }
    
    EXPECT_TRUE(progressCalled);
    EXPECT_EQ(lastLoaded, 20u); // Should eventually reach total
    EXPECT_FALSE(manager.isLoading());
    
    EXPECT_EQ(manager.getLibrarySize(), 20u);
}

TEST_F(PlaylistManagerCoverageTest, AsyncLoadingStop) {
    // Create many files
    for (int i = 0; i < 100; ++i) {
        createMusicFile("song_" + std::to_string(i) + ".mp3");
    }
    
    // Batch 10.
    manager.loadDirectoryAsync(testDir, 10);
    
    EXPECT_TRUE(manager.isLoading());
    
    // Stop immediately
    manager.stopAsyncLoading();
    
    EXPECT_FALSE(manager.isLoading());
    
    // Library might be partial
    size_t size = manager.getLibrarySize();
    EXPECT_GE(size, 10u); // At least first batch
    EXPECT_LE(size, 100u);
}

TEST_F(PlaylistManagerCoverageTest, AsyncLoadingCallbackFirstBatch) {
    for (int i = 0; i < 10; ++i) {
        createMusicFile("song_" + std::to_string(i) + ".mp3");
    }
    
    bool firstBatchCalled = false;
    manager.setLoadProgressCallback([&](size_t loaded, size_t total) {
        if (loaded == 5 && total == 10) {
            firstBatchCalled = true;
        }
    });
    
    manager.loadDirectoryAsync(testDir, 5);
    
    // Provide time for async part too
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    EXPECT_TRUE(firstBatchCalled);
}
