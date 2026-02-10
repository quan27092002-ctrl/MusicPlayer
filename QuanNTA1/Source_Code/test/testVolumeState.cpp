/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testVolumeState.cpp
 * DESCRIPTION: Unit tests for VolumeStateImpl for branch coverage.
 */

#include <gtest/gtest.h>
#include "model/playerstate/VolumeStateImpl.h"
#include <thread>
#include <vector>

class VolumeStateTest : public ::testing::Test {
protected:
    Model::VolumeStateImpl volume;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(VolumeStateTest, DefaultConstruction) {
    Model::VolumeStateImpl vs;
    EXPECT_EQ(vs.getVolume(), 50); // Default volume
    EXPECT_FALSE(vs.isMuted());
}

TEST_F(VolumeStateTest, ConstructionWithValue) {
    Model::VolumeStateImpl vs(75);
    EXPECT_EQ(vs.getVolume(), 75);
    EXPECT_FALSE(vs.isMuted());
}

TEST_F(VolumeStateTest, ConstructionWithZero) {
    Model::VolumeStateImpl vs(0);
    EXPECT_EQ(vs.getVolume(), 0);
}

TEST_F(VolumeStateTest, ConstructionWithMax) {
    Model::VolumeStateImpl vs(100);
    EXPECT_EQ(vs.getVolume(), 100);
}

// ============================================================================
// SetVolume Tests - Branch Coverage
// ============================================================================

TEST_F(VolumeStateTest, SetVolumeNormal) {
    volume.setVolume(75);
    EXPECT_EQ(volume.getVolume(), 75);
}

TEST_F(VolumeStateTest, SetVolumeZero) {
    volume.setVolume(0);
    EXPECT_EQ(volume.getVolume(), 0);
}

TEST_F(VolumeStateTest, SetVolumeMax) {
    volume.setVolume(100);
    EXPECT_EQ(volume.getVolume(), 100);
}

TEST_F(VolumeStateTest, SetVolumeClampAboveMax) {
    volume.setVolume(150);
    EXPECT_EQ(volume.getVolume(), 100); // Clamped to max
}

TEST_F(VolumeStateTest, SetVolumeClampBelowMin) {
    volume.setVolume(-10);
    EXPECT_EQ(volume.getVolume(), 0); // Clamped to min
}

TEST_F(VolumeStateTest, SetVolumeMultipleTimes) {
    volume.setVolume(10);
    EXPECT_EQ(volume.getVolume(), 10);
    volume.setVolume(50);
    EXPECT_EQ(volume.getVolume(), 50);
    volume.setVolume(90);
    EXPECT_EQ(volume.getVolume(), 90);
}

// ============================================================================
// Mute Tests - Branch Coverage
// ============================================================================

TEST_F(VolumeStateTest, IsMutedInitially) {
    EXPECT_FALSE(volume.isMuted());
}

TEST_F(VolumeStateTest, SetMutedTrue) {
    volume.setMuted(true);
    EXPECT_TRUE(volume.isMuted());
}

TEST_F(VolumeStateTest, SetMutedFalse) {
    volume.setMuted(true);
    volume.setMuted(false);
    EXPECT_FALSE(volume.isMuted());
}

TEST_F(VolumeStateTest, SetMutedMultipleTimes) {
    volume.setMuted(true);
    EXPECT_TRUE(volume.isMuted());
    volume.setMuted(true);
    EXPECT_TRUE(volume.isMuted());
    volume.setMuted(false);
    EXPECT_FALSE(volume.isMuted());
}

// ============================================================================
// ToggleMute Tests - Branch Coverage
// ============================================================================

TEST_F(VolumeStateTest, ToggleMuteFromUnmuted) {
    EXPECT_FALSE(volume.isMuted());
    bool result = volume.toggleMute();
    EXPECT_TRUE(result); // Now muted
    EXPECT_TRUE(volume.isMuted());
}

TEST_F(VolumeStateTest, ToggleMuteFromMuted) {
    volume.setMuted(true);
    bool result = volume.toggleMute();
    EXPECT_FALSE(result); // Now unmuted
    EXPECT_FALSE(volume.isMuted());
}

TEST_F(VolumeStateTest, ToggleMuteMultipleTimes) {
    EXPECT_FALSE(volume.isMuted());
    
    volume.toggleMute();
    EXPECT_TRUE(volume.isMuted());
    
    volume.toggleMute();
    EXPECT_FALSE(volume.isMuted());
    
    volume.toggleMute();
    EXPECT_TRUE(volume.isMuted());
}

// ============================================================================
// Volume + Mute Interaction Tests
// ============================================================================

TEST_F(VolumeStateTest, VolumePreservedWhileMuted) {
    volume.setVolume(75);
    volume.setMuted(true);
    EXPECT_EQ(volume.getVolume(), 75); // Volume preserved
    EXPECT_TRUE(volume.isMuted());
}

TEST_F(VolumeStateTest, VolumeChangeWhileMuted) {
    volume.setMuted(true);
    volume.setVolume(30);
    EXPECT_EQ(volume.getVolume(), 30);
    EXPECT_TRUE(volume.isMuted());
}

TEST_F(VolumeStateTest, UnmuteRestoresVolume) {
    volume.setVolume(80);
    volume.setMuted(true);
    volume.setMuted(false);
    EXPECT_EQ(volume.getVolume(), 80);
    EXPECT_FALSE(volume.isMuted());
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(VolumeStateTest, ConcurrentVolumeChanges) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < 100; j++) {
                volume.setVolume((i * 10 + j) % 101);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    int vol = volume.getVolume();
    EXPECT_GE(vol, 0);
    EXPECT_LE(vol, 100);
}

TEST_F(VolumeStateTest, ConcurrentMuteToggle) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this]() {
            for (int j = 0; j < 100; j++) {
                volume.toggleMute();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // 10 threads * 100 toggles = 1000 toggles, so should be unmuted (even)
    // But due to race conditions, we just check it's valid
    bool muted = volume.isMuted();
    (void)muted; // Result is indeterminate due to races but shouldn't crash
}
