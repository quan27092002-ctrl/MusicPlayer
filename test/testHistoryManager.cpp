/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testHistoryManager.cpp
 * DESCRIPTION: Unit tests for HistoryManager for branch coverage.
 */

#include <gtest/gtest.h>
#include "controller/appcontroller/HistoryManager.h"
#include <filesystem>

namespace fs = std::filesystem;

class HistoryManagerTest : public ::testing::Test {
protected:
    std::unique_ptr<Controller::HistoryManagerImpl> historyManager;
    std::string testDir;

    void SetUp() override {
        testDir = "/tmp/history_manager_test_" + std::to_string(getpid());
        fs::create_directories(testDir);
        historyManager = std::make_unique<Controller::HistoryManagerImpl>();
    }

    void TearDown() override {
        historyManager.reset();
        fs::remove_all(testDir);
    }
};

// ============================================================================
// Constructor Tests
// ============================================================================

TEST_F(HistoryManagerTest, Construction) {
    EXPECT_NE(historyManager, nullptr);
}

TEST_F(HistoryManagerTest, InitialHistoryEmpty) {
    EXPECT_EQ(historyManager->getHistorySize(), 0u);
}

TEST_F(HistoryManagerTest, InitialHistoryVector) {
    auto history = historyManager->getHistory();
    EXPECT_TRUE(history.empty());
}

// ============================================================================
// GetHistory Tests
// ============================================================================

TEST_F(HistoryManagerTest, GetHistoryReturnsVector) {
    auto history = historyManager->getHistory();
    EXPECT_EQ(history.size(), 0u);
}

// ============================================================================
// GetHistorySize Tests
// ============================================================================

TEST_F(HistoryManagerTest, GetHistorySizeZero) {
    EXPECT_EQ(historyManager->getHistorySize(), 0u);
}

// ============================================================================
// GetHistoryTrackPath Tests - Branch Coverage
// ============================================================================

TEST_F(HistoryManagerTest, GetHistoryTrackPathOutOfBounds) {
    std::string path = historyManager->getHistoryTrackPath(0);
    EXPECT_TRUE(path.empty());
}

TEST_F(HistoryManagerTest, GetHistoryTrackPathLargeIndex) {
    std::string path = historyManager->getHistoryTrackPath(999);
    EXPECT_TRUE(path.empty());
}

// ============================================================================
// GetHistoryItem Tests - Branch Coverage
// ============================================================================

TEST_F(HistoryManagerTest, GetHistoryItemOutOfBounds) {
    auto item = historyManager->getHistoryItem(0);
    EXPECT_EQ(item, nullptr);
}

TEST_F(HistoryManagerTest, GetHistoryItemLargeIndex) {
    auto item = historyManager->getHistoryItem(999);
    EXPECT_EQ(item, nullptr);
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(HistoryManagerTest, MultipleOutOfBoundsAccess) {
    for (size_t i = 0; i < 100; i++) {
        auto path = historyManager->getHistoryTrackPath(i);
        EXPECT_TRUE(path.empty());
        
        auto item = historyManager->getHistoryItem(i);
        EXPECT_EQ(item, nullptr);
    }
}

// ============================================================================
// ClearHistory Tests
// ============================================================================

TEST_F(HistoryManagerTest, ClearHistoryWhenEmpty) {
    historyManager->clearHistory();
    EXPECT_EQ(historyManager->getHistorySize(), 0u);
}

// ============================================================================
// PopHistory Tests
// ============================================================================

TEST_F(HistoryManagerTest, PopHistoryWhenEmpty) {
    auto item = historyManager->popHistory();
    EXPECT_EQ(item, nullptr);
}
