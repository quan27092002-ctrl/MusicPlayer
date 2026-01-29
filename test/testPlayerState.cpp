/**
 * @file testPlayerState.cpp
 * @brief Unit Tests for PlayerState class using Google Test framework.
 * @details Covers state management, volume control, thread safety, and convenience methods.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include "../src/model/PlayerState.h"

using namespace Model;

/**
 * @brief Test Fixture for PlayerState tests.
 */
class PlayerStateTest : public ::testing::Test {
protected:
    PlayerState state;
};

// ============================================================================
// Initial State Tests
// ============================================================================

TEST_F(PlayerStateTest, DefaultState) {
    EXPECT_EQ(state.getPlaybackState(), PlaybackState::STOPPED);
    EXPECT_EQ(state.getVolume(), 50);
    EXPECT_FALSE(state.isMuted());
    EXPECT_EQ(state.getCurrentPosition(), 0u);
    EXPECT_EQ(state.getCurrentTrackIndex(), -1);
    EXPECT_EQ(state.getRepeatMode(), RepeatMode::NONE);
    EXPECT_FALSE(state.isShuffleEnabled());
    EXPECT_FALSE(state.isPlaying());
}

// ============================================================================
// Playback State Tests
// ============================================================================

TEST_F(PlayerStateTest, SetPlaybackState) {
    state.setPlaybackState(PlaybackState::PLAYING);
    EXPECT_EQ(state.getPlaybackState(), PlaybackState::PLAYING);
    EXPECT_TRUE(state.isPlaying());

    state.setPlaybackState(PlaybackState::PAUSED);
    EXPECT_EQ(state.getPlaybackState(), PlaybackState::PAUSED);
    EXPECT_FALSE(state.isPlaying());

    state.setPlaybackState(PlaybackState::STOPPED);
    EXPECT_EQ(state.getPlaybackState(), PlaybackState::STOPPED);
    EXPECT_FALSE(state.isPlaying());
}

TEST_F(PlayerStateTest, TogglePlayPause) {
    // STOPPED -> PLAYING
    PlaybackState result = state.togglePlayPause();
    EXPECT_EQ(result, PlaybackState::PLAYING);
    EXPECT_TRUE(state.isPlaying());

    // PLAYING -> PAUSED
    result = state.togglePlayPause();
    EXPECT_EQ(result, PlaybackState::PAUSED);
    EXPECT_FALSE(state.isPlaying());

    // PAUSED -> PLAYING
    result = state.togglePlayPause();
    EXPECT_EQ(result, PlaybackState::PLAYING);
}

// ============================================================================
// Volume Tests
// ============================================================================

TEST_F(PlayerStateTest, SetVolume) {
    state.setVolume(75);
    EXPECT_EQ(state.getVolume(), 75);

    state.setVolume(0);
    EXPECT_EQ(state.getVolume(), 0);

    state.setVolume(100);
    EXPECT_EQ(state.getVolume(), 100);
}

TEST_F(PlayerStateTest, VolumeClampingMin) {
    state.setVolume(-50);
    EXPECT_EQ(state.getVolume(), 0);  // Clamped to 0
}

TEST_F(PlayerStateTest, VolumeClampingMax) {
    state.setVolume(200);
    EXPECT_EQ(state.getVolume(), 100);  // Clamped to 100
}

TEST_F(PlayerStateTest, MuteToggle) {
    EXPECT_FALSE(state.isMuted());

    state.setMuted(true);
    EXPECT_TRUE(state.isMuted());

    bool result = state.toggleMute();
    EXPECT_FALSE(result);
    EXPECT_FALSE(state.isMuted());

    result = state.toggleMute();
    EXPECT_TRUE(result);
    EXPECT_TRUE(state.isMuted());
}

// ============================================================================
// Position and Track Tests
// ============================================================================

TEST_F(PlayerStateTest, CurrentPosition) {
    state.setCurrentPosition(120);
    EXPECT_EQ(state.getCurrentPosition(), 120u);

    state.setCurrentPosition(0);
    EXPECT_EQ(state.getCurrentPosition(), 0u);
}

TEST_F(PlayerStateTest, CurrentTrackIndex) {
    state.setCurrentTrackIndex(5);
    EXPECT_EQ(state.getCurrentTrackIndex(), 5);

    state.setCurrentTrackIndex(0);
    EXPECT_EQ(state.getCurrentTrackIndex(), 0);

    state.setCurrentTrackIndex(-1);
    EXPECT_EQ(state.getCurrentTrackIndex(), -1);
}

// ============================================================================
// Playback Mode Tests
// ============================================================================

TEST_F(PlayerStateTest, RepeatMode) {
    state.setRepeatMode(RepeatMode::ONE);
    EXPECT_EQ(state.getRepeatMode(), RepeatMode::ONE);

    state.setRepeatMode(RepeatMode::ALL);
    EXPECT_EQ(state.getRepeatMode(), RepeatMode::ALL);

    state.setRepeatMode(RepeatMode::NONE);
    EXPECT_EQ(state.getRepeatMode(), RepeatMode::NONE);
}

TEST_F(PlayerStateTest, CycleRepeatMode) {
    // NONE -> ONE
    RepeatMode result = state.cycleRepeatMode();
    EXPECT_EQ(result, RepeatMode::ONE);

    // ONE -> ALL
    result = state.cycleRepeatMode();
    EXPECT_EQ(result, RepeatMode::ALL);

    // ALL -> NONE
    result = state.cycleRepeatMode();
    EXPECT_EQ(result, RepeatMode::NONE);
}

TEST_F(PlayerStateTest, ShuffleToggle) {
    EXPECT_FALSE(state.isShuffleEnabled());

    state.setShuffleEnabled(true);
    EXPECT_TRUE(state.isShuffleEnabled());

    bool result = state.toggleShuffle();
    EXPECT_FALSE(result);
    EXPECT_FALSE(state.isShuffleEnabled());
}

// ============================================================================
// Reset Test
// ============================================================================

TEST_F(PlayerStateTest, Reset) {
    // Modify all state
    state.setPlaybackState(PlaybackState::PLAYING);
    state.setVolume(80);
    state.setMuted(true);
    state.setCurrentPosition(300);
    state.setCurrentTrackIndex(5);
    state.setRepeatMode(RepeatMode::ALL);
    state.setShuffleEnabled(true);

    // Reset
    state.reset();

    // Verify all back to defaults
    EXPECT_EQ(state.getPlaybackState(), PlaybackState::STOPPED);
    EXPECT_EQ(state.getVolume(), 50);
    EXPECT_FALSE(state.isMuted());
    EXPECT_EQ(state.getCurrentPosition(), 0u);
    EXPECT_EQ(state.getCurrentTrackIndex(), -1);
    EXPECT_EQ(state.getRepeatMode(), RepeatMode::NONE);
    EXPECT_FALSE(state.isShuffleEnabled());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(PlayerStateTest, ConcurrentVolumeAccess) {
    const int NUM_THREADS = 4;
    const int ITERATIONS = 1000;
    std::vector<std::thread> threads;

    // Multiple threads changing volume
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this, t]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                state.setVolume((t * ITERATIONS + i) % 101);
                volatile int v = state.getVolume();  // Force read
                (void)v;
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // Volume should be in valid range
    EXPECT_GE(state.getVolume(), 0);
    EXPECT_LE(state.getVolume(), 100);
}

TEST_F(PlayerStateTest, ConcurrentPlaybackToggle) {
    const int NUM_THREADS = 4;
    const int ITERATIONS = 500;
    std::vector<std::thread> threads;

    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([this]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                state.togglePlayPause();
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    // State should be valid (no crash, valid enum value)
    PlaybackState finalState = state.getPlaybackState();
    EXPECT_TRUE(finalState == PlaybackState::STOPPED ||
                finalState == PlaybackState::PLAYING ||
                finalState == PlaybackState::PAUSED);
}

TEST_F(PlayerStateTest, ConcurrentMixedOperations) {
    const int ITERATIONS = 500;
    std::atomic<bool> running{true};

    // Thread 1: Volume changes
    std::thread volumeThread([this, &running]() {
        while (running) {
            for (int v = 0; v <= 100; v += 10) {
                state.setVolume(v);
            }
        }
    });

    // Thread 2: Playback toggle
    std::thread playThread([this]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            state.togglePlayPause();
        }
    });

    // Thread 3: Position updates
    std::thread posThread([this]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            state.setCurrentPosition(i);
        }
    });

    // Thread 4: Mode cycling
    std::thread modeThread([this]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            state.cycleRepeatMode();
            state.toggleShuffle();
        }
    });

    playThread.join();
    posThread.join();
    modeThread.join();
    
    running = false;
    volumeThread.join();

    // All operations completed without crash
    SUCCEED();
}
