/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testTrackPosition.cpp
 * DESCRIPTION: Unit tests for TrackPositionImpl class for branch coverage.
 */

#include <gtest/gtest.h>
#include "model/playerstate/TrackPositionImpl.h"
#include <thread>
#include <vector>

class TrackPositionTest : public ::testing::Test {
protected:
    Model::TrackPositionImpl position;
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(TrackPositionTest, DefaultConstruction) {
    Model::TrackPositionImpl tp;
    EXPECT_EQ(tp.getCurrentPosition(), 0u);
    EXPECT_EQ(tp.getPlaybackVersion(), 0u);
}

TEST_F(TrackPositionTest, ConstructionWithInitialPosition) {
    Model::TrackPositionImpl tp(12345);
    EXPECT_EQ(tp.getCurrentPosition(), 12345u);
    EXPECT_EQ(tp.getPlaybackVersion(), 0u);
}

TEST_F(TrackPositionTest, ConstructionWithZeroPosition) {
    Model::TrackPositionImpl tp(0);
    EXPECT_EQ(tp.getCurrentPosition(), 0u);
}

TEST_F(TrackPositionTest, ConstructionWithMaxPosition) {
    Model::TrackPositionImpl tp(UINT32_MAX);
    EXPECT_EQ(tp.getCurrentPosition(), UINT32_MAX);
}

// ============================================================================
// GetCurrentPosition Tests
// ============================================================================

TEST_F(TrackPositionTest, GetCurrentPositionInitial) {
    EXPECT_EQ(position.getCurrentPosition(), 0u);
}

TEST_F(TrackPositionTest, GetCurrentPositionAfterSet) {
    position.setCurrentPosition(5000);
    EXPECT_EQ(position.getCurrentPosition(), 5000u);
}

// ============================================================================
// SetCurrentPosition Tests
// ============================================================================

TEST_F(TrackPositionTest, SetCurrentPosition) {
    position.setCurrentPosition(10000);
    EXPECT_EQ(position.getCurrentPosition(), 10000u);
}

TEST_F(TrackPositionTest, SetCurrentPositionZero) {
    position.setCurrentPosition(5000);
    position.setCurrentPosition(0);
    EXPECT_EQ(position.getCurrentPosition(), 0u);
}

TEST_F(TrackPositionTest, SetCurrentPositionMultipleTimes) {
    for (uint32_t i = 0; i < 100; i++) {
        position.setCurrentPosition(i * 1000);
        EXPECT_EQ(position.getCurrentPosition(), i * 1000);
    }
}

TEST_F(TrackPositionTest, SetCurrentPositionLargeValue) {
    position.setCurrentPosition(3600000); // 1 hour in ms
    EXPECT_EQ(position.getCurrentPosition(), 3600000u);
}

// ============================================================================
// GetPlaybackVersion Tests
// ============================================================================

TEST_F(TrackPositionTest, GetPlaybackVersionInitial) {
    EXPECT_EQ(position.getPlaybackVersion(), 0u);
}

TEST_F(TrackPositionTest, GetPlaybackVersionAfterIncrement) {
    position.incrementPlaybackVersion();
    EXPECT_EQ(position.getPlaybackVersion(), 1u);
}

// ============================================================================
// IncrementPlaybackVersion Tests
// ============================================================================

TEST_F(TrackPositionTest, IncrementPlaybackVersionOnce) {
    position.incrementPlaybackVersion();
    EXPECT_EQ(position.getPlaybackVersion(), 1u);
}

TEST_F(TrackPositionTest, IncrementPlaybackVersionMultiple) {
    for (int i = 0; i < 10; i++) {
        position.incrementPlaybackVersion();
    }
    EXPECT_EQ(position.getPlaybackVersion(), 10u);
}

TEST_F(TrackPositionTest, IncrementPlaybackVersionManyTimes) {
    for (int i = 0; i < 1000; i++) {
        position.incrementPlaybackVersion();
    }
    EXPECT_EQ(position.getPlaybackVersion(), 1000u);
}

// ============================================================================
// Reset Tests
// ============================================================================

TEST_F(TrackPositionTest, ResetFromDefault) {
    position.reset();
    EXPECT_EQ(position.getCurrentPosition(), 0u);
    EXPECT_EQ(position.getPlaybackVersion(), 0u);
}

TEST_F(TrackPositionTest, ResetAfterModifications) {
    position.setCurrentPosition(5000);
    position.incrementPlaybackVersion();
    position.incrementPlaybackVersion();
    
    EXPECT_EQ(position.getCurrentPosition(), 5000u);
    EXPECT_EQ(position.getPlaybackVersion(), 2u);
    
    position.reset();
    
    EXPECT_EQ(position.getCurrentPosition(), 0u);
    EXPECT_EQ(position.getPlaybackVersion(), 0u);
}

TEST_F(TrackPositionTest, ResetMultipleTimes) {
    position.setCurrentPosition(1000);
    position.reset();
    position.setCurrentPosition(2000);
    position.reset();
    
    EXPECT_EQ(position.getCurrentPosition(), 0u);
    EXPECT_EQ(position.getPlaybackVersion(), 0u);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(TrackPositionTest, ConcurrentPositionUpdates) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this, i]() {
            for (int j = 0; j < 100; j++) {
                position.setCurrentPosition(i * 1000 + j);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Position should be some valid value
    EXPECT_GE(position.getCurrentPosition(), 0u);
}

TEST_F(TrackPositionTest, ConcurrentVersionIncrements) {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([this]() {
            for (int j = 0; j < 100; j++) {
                position.incrementPlaybackVersion();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Total increments should be 10 * 100 = 1000
    EXPECT_EQ(position.getPlaybackVersion(), 1000u);
}

TEST_F(TrackPositionTest, ConcurrentMixedOperations) {
    std::vector<std::thread> threads;
    
    // Thread 1: Set positions
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; i++) {
            position.setCurrentPosition(i * 100);
        }
    });
    
    // Thread 2: Increment versions
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; i++) {
            position.incrementPlaybackVersion();
        }
    });
    
    // Thread 3: Read positions
    threads.emplace_back([this]() {
        for (int i = 0; i < 100; i++) {
            auto pos = position.getCurrentPosition();
            auto ver = position.getPlaybackVersion();
            (void)pos;
            (void)ver;
        }
    });
    
    for (auto& t : threads) {
        t.join();
    }
    
    EXPECT_EQ(position.getPlaybackVersion(), 100u);
}
