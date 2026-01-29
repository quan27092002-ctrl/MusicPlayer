/**
 * @file testAudioPlayer.cpp
 * @brief Unit Tests for AudioPlayer class.
 * @details Tests basic functionality without actually playing audio.
 * @note SDL audio tests may hang on systems without audio device.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include "../src/controller/AudioPlayer.h"

using namespace Controller;

// Path to test assets (absolute path)
const std::string TEST_WAV_PATH = "/home/quan/QuanNTA1_MockProject/test/assets/test_tone.wav";
const std::string TEST_MP3_PATH = "/home/quan/QuanNTA1_MockProject/test/assets/test_tone.mp3";
const std::string INVALID_PATH = "/home/quan/QuanNTA1_MockProject/test/assets/nonexistent.mp3";

// ============================================================================
// Basic Tests (No SDL initialization required)
// ============================================================================

TEST(AudioPlayerBasicTest, Construction) {
    AudioPlayer player;
    // Should not crash on construction
    SUCCEED();
}

TEST(AudioPlayerBasicTest, VolumeBeforeInit) {
    AudioPlayer player;
    
    // Volume should work even before initialize
    player.setVolume(75);
    EXPECT_EQ(player.getVolume(), 75);
}

TEST(AudioPlayerBasicTest, VolumeClamping) {
    AudioPlayer player;
    
    player.setVolume(-10);
    EXPECT_EQ(player.getVolume(), 0);
    
    player.setVolume(150);
    EXPECT_EQ(player.getVolume(), 100);
}

TEST(AudioPlayerBasicTest, StateBeforeInit) {
    AudioPlayer player;
    
    EXPECT_EQ(player.getState(), AudioState::IDLE);
    EXPECT_FALSE(player.isLoaded());
    EXPECT_FALSE(player.isPlaying());
    EXPECT_EQ(player.getPosition(), 0u);
    EXPECT_EQ(player.getDuration(), 0u);
}

TEST(AudioPlayerBasicTest, LoadBeforeInit) {
    AudioPlayer player;
    
    // Should fail gracefully without crashing
    EXPECT_FALSE(player.load(TEST_WAV_PATH));
}

TEST(AudioPlayerBasicTest, PlayBeforeInit) {
    AudioPlayer player;
    
    // Should not crash
    player.play();
    player.pause();
    player.stop();
    player.seek(100);
    
    SUCCEED();
}

TEST(AudioPlayerBasicTest, CallbackSetting) {
    AudioPlayer player;
    
    bool callbackSet = false;
    player.setCallback([&](AudioState, uint32_t) {
        callbackSet = true;
    });
    
    // Callback is set but won't be called without actual events
    SUCCEED();
}

// ============================================================================
// SDL Initialization Tests (Skipped if no audio device)
// ============================================================================

class AudioPlayerSDLTest : public ::testing::Test {
protected:
    AudioPlayer player;
    bool sdlAvailable = false;

    void SetUp() override {
        // Try to initialize - may fail on headless systems
        sdlAvailable = player.initialize();
        if (!sdlAvailable) {
            GTEST_SKIP() << "SDL audio not available on this system";
        }
    }

    void TearDown() override {
        if (sdlAvailable) {
            player.shutdown();
        }
    }
};

TEST_F(AudioPlayerSDLTest, Initialize) {
    EXPECT_EQ(player.getState(), AudioState::IDLE);
}

TEST_F(AudioPlayerSDLTest, DoubleInitialize) {
    EXPECT_TRUE(player.initialize());
}

TEST_F(AudioPlayerSDLTest, LoadWavFile) {
    EXPECT_TRUE(player.load(TEST_WAV_PATH));
    EXPECT_EQ(player.getState(), AudioState::LOADED);
    EXPECT_TRUE(player.isLoaded());
}

TEST_F(AudioPlayerSDLTest, LoadMp3File) {
    EXPECT_TRUE(player.load(TEST_MP3_PATH));
    EXPECT_EQ(player.getState(), AudioState::LOADED);
    EXPECT_TRUE(player.isLoaded());
}

TEST_F(AudioPlayerSDLTest, LoadInvalidFile) {
    EXPECT_FALSE(player.load(INVALID_PATH));
    EXPECT_EQ(player.getState(), AudioState::ERROR);
}

TEST_F(AudioPlayerSDLTest, Unload) {
    player.load(TEST_WAV_PATH);
    player.unload();
    
    EXPECT_EQ(player.getState(), AudioState::IDLE);
    EXPECT_FALSE(player.isLoaded());
}

TEST_F(AudioPlayerSDLTest, VolumeAfterInit) {
    player.setVolume(80);
    EXPECT_EQ(player.getVolume(), 80);
}

TEST_F(AudioPlayerSDLTest, ShutdownCleanup) {
    player.load(TEST_WAV_PATH);
    player.shutdown();
    
    EXPECT_EQ(player.getState(), AudioState::IDLE);
}
