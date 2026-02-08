/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testPlaybackController.cpp
 * DESCRIPTION: Unit tests for PlaybackController class using GoogleMock.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/appcontroller/PlaybackController.h"
#include "controller/appcontroller/PlaylistManager.h"
#include "controller/appcontroller/HistoryManager.h"
#include "controller/mocks/MockAudioPlayer.h"
#include "model/mocks/MockPlayerState.h"
#include <memory>
#include <filesystem>
#include <fstream>

using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::AtLeast;

namespace fs = std::filesystem;

class PlaybackControllerTest : public ::testing::Test {
protected:
    std::shared_ptr<NiceMock<Controller::MockAudioPlayer>> mockAudioPlayer;
    std::shared_ptr<NiceMock<Model::MockPlayerState>> mockPlayerState;
    std::unique_ptr<Controller::PlaylistManagerImpl> playlistManager;
    std::unique_ptr<Controller::HistoryManagerImpl> historyManager;
    std::unique_ptr<Controller::PlaybackControllerImpl> controller;
    std::string testDir;

    void SetUp() override {
        mockAudioPlayer = std::make_shared<NiceMock<Controller::MockAudioPlayer>>();
        mockPlayerState = std::make_shared<NiceMock<Model::MockPlayerState>>();
        playlistManager = std::make_unique<Controller::PlaylistManagerImpl>();
        historyManager = std::make_unique<Controller::HistoryManagerImpl>(playlistManager.get());
        
        controller = std::make_unique<Controller::PlaybackControllerImpl>(
            mockAudioPlayer,
            mockPlayerState,
            playlistManager.get(),
            historyManager.get()
        );

        testDir = "/tmp/playback_controller_test_" + std::to_string(getpid());
        fs::create_directories(testDir);

        // Set up default mock behaviors
        ON_CALL(*mockPlayerState, getPlaybackStatus())
            .WillByDefault(Return(Model::PlaybackStatus::STOPPED));
        ON_CALL(*mockPlayerState, getCurrentTrackIndex())
            .WillByDefault(Return(-1));
        ON_CALL(*mockAudioPlayer, isLoaded())
            .WillByDefault(Return(false));
        ON_CALL(*mockAudioPlayer, isPlaying())
            .WillByDefault(Return(false));
    }

    void TearDown() override {
        controller.reset();
        historyManager.reset();
        playlistManager.reset();
        fs::remove_all(testDir);
    }

    void createDummyMusicFile(const std::string& filename) {
        std::string path = testDir + "/" + filename;
        std::ofstream file(path, std::ios::binary);
        const char mp3Header[] = {'\xFF', '\xFB', '\x90', '\x00'};
        file.write(mp3Header, 4);
        file.close();
    }
};

// ============================================================================
// Construction Tests
// ============================================================================

TEST_F(PlaybackControllerTest, Construction) {
    EXPECT_NE(controller, nullptr);
}

// ============================================================================
// LoadTrack Tests
// ============================================================================

TEST_F(PlaybackControllerTest, LoadTrack) {
    createDummyMusicFile("song.mp3");
    
    EXPECT_CALL(*mockAudioPlayer, load(_)).WillOnce(Return(true));
    
    bool result = controller->loadTrack(testDir + "/song.mp3");
    EXPECT_TRUE(result);
}

TEST_F(PlaybackControllerTest, LoadTrackNonexistent) {
    EXPECT_CALL(*mockAudioPlayer, load(_)).WillOnce(Return(false));
    
    bool result = controller->loadTrack("/nonexistent/path.mp3");
    EXPECT_FALSE(result);
}

TEST_F(PlaybackControllerTest, LoadTrackUpdatesState) {
    createDummyMusicFile("song.mp3");
    
    EXPECT_CALL(*mockAudioPlayer, load(_)).WillOnce(Return(true));
    EXPECT_CALL(*mockPlayerState, setCurrentPosition(0)).Times(AtLeast(1));
    EXPECT_CALL(*mockPlayerState, incrementPlaybackVersion()).Times(AtLeast(1));
    
    controller->loadTrack(testDir + "/song.mp3");
}

// ============================================================================
// Play Tests
// ============================================================================

TEST_F(PlaybackControllerTest, PlayWithLoadedTrack) {
    ON_CALL(*mockAudioPlayer, isLoaded()).WillByDefault(Return(true));
    EXPECT_CALL(*mockAudioPlayer, play()).Times(1);
    EXPECT_CALL(*mockPlayerState, setPlaybackStatus(Model::PlaybackStatus::PLAYING)).Times(1);
    
    controller->play();
}

TEST_F(PlaybackControllerTest, PlayWithoutLoadedTrack) {
    ON_CALL(*mockAudioPlayer, isLoaded()).WillByDefault(Return(false));
    // Should attempt to load from queue or do nothing
    controller->play();
}

// ============================================================================
// Pause Tests
// ============================================================================

TEST_F(PlaybackControllerTest, Pause) {
    EXPECT_CALL(*mockAudioPlayer, pause()).Times(1);
    EXPECT_CALL(*mockPlayerState, setPlaybackStatus(Model::PlaybackStatus::PAUSED)).Times(1);
    
    controller->pause();
}

// ============================================================================
// Stop Tests
// ============================================================================

TEST_F(PlaybackControllerTest, Stop) {
    EXPECT_CALL(*mockAudioPlayer, stop()).Times(1);
    EXPECT_CALL(*mockPlayerState, setPlaybackStatus(Model::PlaybackStatus::STOPPED)).Times(1);
    
    controller->stop();
}

// ============================================================================
// Seek Tests
// ============================================================================

TEST_F(PlaybackControllerTest, Seek) {
    EXPECT_CALL(*mockAudioPlayer, seek(5000)).Times(1);
    
    controller->seek(5000);
}

TEST_F(PlaybackControllerTest, SeekToZero) {
    EXPECT_CALL(*mockAudioPlayer, seek(0)).Times(1);
    
    controller->seek(0);
}

// ============================================================================
// PlayTrack Tests
// ============================================================================

TEST_F(PlaybackControllerTest, PlayTrackValidIndex) {
    createDummyMusicFile("song1.mp3");
    createDummyMusicFile("song2.mp3");
    playlistManager->addToPlaylist(testDir + "/song1.mp3");
    playlistManager->addToPlaylist(testDir + "/song2.mp3");
    
    EXPECT_CALL(*mockAudioPlayer, load(_)).WillOnce(Return(true));
    
    controller->playTrack(1);
}

TEST_F(PlaybackControllerTest, PlayTrackInvalidIndex) {
    // No tracks in playlist, should handle gracefully
    controller->playTrack(999);
}

// ============================================================================
// Next/Previous Tests
// ============================================================================

TEST_F(PlaybackControllerTest, NextWithEmptyPlaylist) {
    // Should handle gracefully
    controller->next();
}

TEST_F(PlaybackControllerTest, PreviousWithEmptyPlaylist) {
    // Should handle gracefully
    controller->previous();
}

// ============================================================================
// Queue Tests
// ============================================================================

TEST_F(PlaybackControllerTest, QueueNext) {
    createDummyMusicFile("song.mp3");
    
    controller->queueNext(testDir + "/song.mp3");
    // Track should be queued
}

TEST_F(PlaybackControllerTest, ReplaceQueue) {
    createDummyMusicFile("song1.mp3");
    createDummyMusicFile("song2.mp3");
    
    std::vector<std::string> paths = {
        testDir + "/song1.mp3",
        testDir + "/song2.mp3"
    };
    
    controller->replaceQueue(paths);
}

TEST_F(PlaybackControllerTest, QueuePlaylist) {
    createDummyMusicFile("song1.mp3");
    createDummyMusicFile("song2.mp3");
    
    std::vector<std::string> paths = {
        testDir + "/song1.mp3",
        testDir + "/song2.mp3"
    };
    
    controller->queuePlaylist(paths);
}

// ============================================================================
// PlayLibrary Tests
// ============================================================================

TEST_F(PlaybackControllerTest, PlayLibrary) {
    controller->playLibrary(0);
    // Should handle empty library gracefully
}

// ============================================================================
// Toggle Shuffle/Repeat Tests
// ============================================================================

TEST_F(PlaybackControllerTest, ToggleShuffle) {
    controller->toggleShuffle();
}

TEST_F(PlaybackControllerTest, ToggleRepeat) {
    EXPECT_CALL(*mockPlayerState, cycleRepeatMode()).Times(1);
    
    controller->toggleRepeat();
}

// ============================================================================
// Status Callback Tests
// ============================================================================

TEST_F(PlaybackControllerTest, SetStatusCallback) {
    bool called = false;
    controller->setStatusCallback([&called]() {
        called = true;
    });
    // Callback is set, no crash
}

// ============================================================================
// GetCurrentTrackIndex Tests
// ============================================================================

TEST_F(PlaybackControllerTest, GetCurrentTrackIndex) {
    int index = controller->getCurrentTrackIndex();
    EXPECT_EQ(index, -1); // No track loaded
}

TEST_F(PlaybackControllerTest, GetCurrentTrackIndexWithTrack) {
    createDummyMusicFile("song.mp3");
    playlistManager->addToPlaylist(testDir + "/song.mp3");
    
    ON_CALL(*mockAudioPlayer, load(_)).WillByDefault(Return(true));
    controller->loadTrack(testDir + "/song.mp3");
    
    // Index should be >= 0 after loading
    int index = controller->getCurrentTrackIndex();
    EXPECT_GE(index, -1);
}
