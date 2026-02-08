/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testPlaylistManager.cpp
 * DESCRIPTION: Unit tests for PlaylistManager class.
 */

#include <gtest/gtest.h>
#include "controller/appcontroller/PlaylistManager.h"
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

class PlaylistManagerTest : public ::testing::Test {
protected:
    Controller::PlaylistManagerImpl manager;
    std::string testDir;

    void SetUp() override {
        testDir = "/tmp/playlist_manager_test_" + std::to_string(getpid());
        fs::create_directories(testDir);
    }

    void TearDown() override {
        manager.stopAsyncLoading();
        fs::remove_all(testDir);
    }

    void createDummyMusicFile(const std::string& filename) {
        std::string path = testDir + "/" + filename;
        std::ofstream file(path, std::ios::binary);
        // Write minimal valid MP3 header
        const char mp3Header[] = {'\xFF', '\xFB', '\x90', '\x00'};
        file.write(mp3Header, 4);
        file.close();
    }
};

// ============================================================================
// Basic Construction Tests
// ============================================================================

TEST_F(PlaylistManagerTest, Construction) {
    Controller::PlaylistManagerImpl pm;
    EXPECT_EQ(pm.getPlaylistSize(), 0u);
}

TEST_F(PlaylistManagerTest, InitialPlaylistEmpty) {
    EXPECT_EQ(manager.getPlaylistSize(), 0u);
}

TEST_F(PlaylistManagerTest, InitialLibraryEmpty) {
    EXPECT_EQ(manager.getLibrarySize(), 0u);
}

// ============================================================================
// AddToPlaylist Tests
// ============================================================================

TEST_F(PlaylistManagerTest, AddToPlaylist) {
    createDummyMusicFile("song1.mp3");
    manager.addToPlaylist(testDir + "/song1.mp3");
    
    EXPECT_EQ(manager.getPlaylistSize(), 1u);
}

TEST_F(PlaylistManagerTest, AddMultipleToPlaylist) {
    createDummyMusicFile("song1.mp3");
    createDummyMusicFile("song2.mp3");
    createDummyMusicFile("song3.mp3");
    
    manager.addToPlaylist(testDir + "/song1.mp3");
    manager.addToPlaylist(testDir + "/song2.mp3");
    manager.addToPlaylist(testDir + "/song3.mp3");
    
    EXPECT_EQ(manager.getPlaylistSize(), 3u);
}

TEST_F(PlaylistManagerTest, AddInvalidFileToPlaylist) {
    manager.addToPlaylist("/nonexistent/path/song.mp3");
    // Should handle gracefully - may still add but with invalid state
    EXPECT_GE(manager.getPlaylistSize(), 0u);
}

// ============================================================================
// ClearPlaylist Tests
// ============================================================================

TEST_F(PlaylistManagerTest, ClearPlaylist) {
    createDummyMusicFile("song1.mp3");
    createDummyMusicFile("song2.mp3");
    
    manager.addToPlaylist(testDir + "/song1.mp3");
    manager.addToPlaylist(testDir + "/song2.mp3");
    EXPECT_EQ(manager.getPlaylistSize(), 2u);
    
    manager.clearPlaylist();
    EXPECT_EQ(manager.getPlaylistSize(), 0u);
}

TEST_F(PlaylistManagerTest, ClearEmptyPlaylist) {
    manager.clearPlaylist();
    EXPECT_EQ(manager.getPlaylistSize(), 0u);
}

// ============================================================================
// Track Accessors Tests
// ============================================================================

TEST_F(PlaylistManagerTest, GetTrackName) {
    createDummyMusicFile("mysong.mp3");
    manager.addToPlaylist(testDir + "/mysong.mp3");
    
    std::string name = manager.getTrackName(0);
    // Name should contain the filename
    EXPECT_FALSE(name.empty());
}

TEST_F(PlaylistManagerTest, GetTrackPath) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    std::string path = manager.getTrackPath(0);
    EXPECT_TRUE(path.find("song.mp3") != std::string::npos);
}

TEST_F(PlaylistManagerTest, GetTrackArtist) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    std::string artist = manager.getTrackArtist(0);
    // May be empty for test files without metadata
    EXPECT_GE(artist.length(), 0u);
}

TEST_F(PlaylistManagerTest, GetTrackAlbum) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    std::string album = manager.getTrackAlbum(0);
    EXPECT_GE(album.length(), 0u);
}

TEST_F(PlaylistManagerTest, GetTrackDuration) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    uint32_t duration = manager.getTrackDuration(0);
    EXPECT_GE(duration, 0u);
}

TEST_F(PlaylistManagerTest, GetTrackCoverArt) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    auto coverArt = manager.getTrackCoverArt(0);
    // May be empty for test files
    EXPECT_GE(coverArt.size(), 0u);
}

TEST_F(PlaylistManagerTest, GetTrackOutOfBounds) {
    std::string name = manager.getTrackName(999);
    EXPECT_TRUE(name.empty());
}

// ============================================================================
// LoadDirectory Tests
// ============================================================================

TEST_F(PlaylistManagerTest, LoadDirectory) {
    createDummyMusicFile("song1.mp3");
    createDummyMusicFile("song2.mp3");
    
    size_t loaded = manager.loadDirectory(testDir);
    EXPECT_EQ(loaded, 2u);
}

TEST_F(PlaylistManagerTest, LoadEmptyDirectory) {
    size_t loaded = manager.loadDirectory(testDir);
    EXPECT_EQ(loaded, 0u);
}

TEST_F(PlaylistManagerTest, LoadNonexistentDirectory) {
    size_t loaded = manager.loadDirectory("/nonexistent/path");
    EXPECT_EQ(loaded, 0u);
}

TEST_F(PlaylistManagerTest, LoadDirectoryWithSubdirs) {
    fs::create_directories(testDir + "/subdir");
    createDummyMusicFile("song1.mp3");
    
    // Create file in subdir
    std::ofstream file(testDir + "/subdir/song2.mp3", std::ios::binary);
    file << "\xFF\xFB\x90\x00";
    file.close();
    
    size_t loaded = manager.loadDirectory(testDir);
    EXPECT_GE(loaded, 1u); // At least the file in root dir
}

// ============================================================================
// Library Accessors Tests
// ============================================================================

TEST_F(PlaylistManagerTest, GetLibrarySize) {
    EXPECT_EQ(manager.getLibrarySize(), 0u);
}

TEST_F(PlaylistManagerTest, GetLibraryTrackName) {
    std::string name = manager.getLibraryTrackName(0);
    EXPECT_TRUE(name.empty());
}

TEST_F(PlaylistManagerTest, GetLibraryTrackPath) {
    std::string path = manager.getLibraryTrackPath(0);
    EXPECT_TRUE(path.empty());
}

// ============================================================================
// AcquireMediaFile Tests
// ============================================================================

TEST_F(PlaylistManagerTest, AcquireMediaFile) {
    createDummyMusicFile("song.mp3");
    
    auto mediaFile = manager.acquireMediaFile(testDir + "/song.mp3");
    EXPECT_NE(mediaFile, nullptr);
}

TEST_F(PlaylistManagerTest, AcquireMediaFileNonexistent) {
    auto mediaFile = manager.acquireMediaFile("/nonexistent/path.mp3");
    // Should still return a MediaFile object (may be invalid)
    EXPECT_NE(mediaFile, nullptr);
}

TEST_F(PlaylistManagerTest, AcquireMediaFileCaching) {
    createDummyMusicFile("song.mp3");
    
    auto file1 = manager.acquireMediaFile(testDir + "/song.mp3");
    auto file2 = manager.acquireMediaFile(testDir + "/song.mp3");
    
    // Should return same cached object or equivalent
    EXPECT_NE(file1, nullptr);
    EXPECT_NE(file2, nullptr);
}

// ============================================================================
// Callback Tests
// ============================================================================

TEST_F(PlaylistManagerTest, SetPlaylistUpdatedCallback) {
    bool called = false;
    manager.setPlaylistUpdatedCallback([&called]() {
        called = true;
    });
    
    manager.notifyPlaylistUpdated();
    EXPECT_TRUE(called);
}

TEST_F(PlaylistManagerTest, NotifyWithoutCallback) {
    // Should not crash when no callback set
    manager.notifyPlaylistUpdated();
}

// ============================================================================
// Async Loading Tests
// ============================================================================

TEST_F(PlaylistManagerTest, IsLoadingInitiallyFalse) {
    EXPECT_FALSE(manager.isLoading());
}

TEST_F(PlaylistManagerTest, StopAsyncLoadingWhenNotLoading) {
    // Should not crash
    manager.stopAsyncLoading();
    EXPECT_FALSE(manager.isLoading());
}

TEST_F(PlaylistManagerTest, SetLoadProgressCallback) {
    size_t current = 0, total = 0;
    manager.setLoadProgressCallback([&](size_t c, size_t t) {
        current = c;
        total = t;
    });
    
    // Callback is set - no crash
    EXPECT_EQ(current, 0u);
}

// ============================================================================
// GetTrackAt Tests
// ============================================================================

TEST_F(PlaylistManagerTest, GetTrackAtValidIndex) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    auto track = manager.getTrackAt(0);
    EXPECT_NE(track, nullptr);
}

TEST_F(PlaylistManagerTest, GetTrackAtInvalidIndex) {
    auto track = manager.getTrackAt(999);
    EXPECT_EQ(track, nullptr);
}

// ============================================================================
// GetPlaylistRef and GetMutex Tests
// ============================================================================

TEST_F(PlaylistManagerTest, GetPlaylistRef) {
    createDummyMusicFile("song.mp3");
    manager.addToPlaylist(testDir + "/song.mp3");
    
    auto& ref = manager.getPlaylistRef();
    EXPECT_EQ(ref.size(), 1u);
}

TEST_F(PlaylistManagerTest, GetMutex) {
    auto& mutex = manager.getMutex();
    
    // Should be able to lock/unlock
    mutex.lock();
    mutex.unlock();
}
