/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/testHistoryManager.cpp
 * DESCRIPTION: Unit tests for HistoryManager for branch coverage.
 */

#include <gtest/gtest.h>
#include "controller/appcontroller/HistoryManager.h"
#include "model/MediaFile.h"
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

// ============================================================================
// Functional Tests (Coverage Improvements)
// ============================================================================

TEST_F(HistoryManagerTest, PushHistoryAddsItem) {
    auto file = std::make_shared<Model::MediaFile>("song.mp3", "/path/song.mp3");
    historyManager->pushHistory(file);
    
    EXPECT_EQ(historyManager->getHistorySize(), 1u);
    EXPECT_EQ(historyManager->getHistoryTrackPath(0), "/path/song.mp3");
    
    auto retrieved = historyManager->getHistoryItem(0);
    EXPECT_EQ(retrieved, file);
}

TEST_F(HistoryManagerTest, PushHistoryNullIgnored) {
    historyManager->pushHistory(nullptr);
    EXPECT_EQ(historyManager->getHistorySize(), 0u);
}

TEST_F(HistoryManagerTest, PushHistoryDuplicateIgnored) {
    auto file = std::make_shared<Model::MediaFile>("song.mp3", "/path/song.mp3");
    historyManager->pushHistory(file);
    historyManager->pushHistory(file); // Duplicate
    
    EXPECT_EQ(historyManager->getHistorySize(), 1u);
}

TEST_F(HistoryManagerTest, PushHistoryMaxLimitEnforced) {
    // Push 55 items (Limit is 50)
    for (int i = 0; i < 55; ++i) {
        std::string name = "song" + std::to_string(i) + ".mp3";
        auto file = std::make_shared<Model::MediaFile>(name, "/path/" + name);
        historyManager->pushHistory(file);
    }
    
    EXPECT_EQ(historyManager->getHistorySize(), 50u);
    
    // First item should be index 5 (since 0-4 were removed)
    // Actually, history stack stores [5...54]
    // getHistoryItem(0) -> index 5
    EXPECT_EQ(historyManager->getHistoryTrackPath(0), "/path/song5.mp3");
    EXPECT_EQ(historyManager->getHistoryTrackPath(49), "/path/song54.mp3");
}

TEST_F(HistoryManagerTest, PopHistoryReturnsLastItem) {
    auto file1 = std::make_shared<Model::MediaFile>("1.mp3", "/path/1.mp3");
    auto file2 = std::make_shared<Model::MediaFile>("2.mp3", "/path/2.mp3");
    
    historyManager->pushHistory(file1);
    historyManager->pushHistory(file2);
    
    auto popped = historyManager->popHistory();
    EXPECT_EQ(popped, file2);
    EXPECT_EQ(historyManager->getHistorySize(), 1u);
    
    popped = historyManager->popHistory();
    EXPECT_EQ(popped, file1);
    EXPECT_EQ(historyManager->getHistorySize(), 0u);
}

TEST_F(HistoryManagerTest, GetHistoryIndicesWithPlaylist) {
    std::list<std::shared_ptr<Model::MediaFile>> playlist;
    std::mutex playlistMutex;
    
    auto f1 = std::make_shared<Model::MediaFile>("1.mp3", "/p/1.mp3");
    auto f2 = std::make_shared<Model::MediaFile>("2.mp3", "/p/2.mp3");
    auto f3 = std::make_shared<Model::MediaFile>("3.mp3", "/p/3.mp3");
    
    playlist.push_back(f1); // Index 0
    playlist.push_back(f2); // Index 1
    playlist.push_back(f3); // Index 2
    
    historyManager->setPlaylistRef(&playlist, &playlistMutex);
    
    // Add 1 and 3 to history
    historyManager->pushHistory(f1);
    historyManager->pushHistory(f3);
    
    auto indices = historyManager->getHistory();
    ASSERT_EQ(indices.size(), 2u);
    EXPECT_EQ(indices[0], 0); // f1 is at index 0
    EXPECT_EQ(indices[1], 2); // f3 is at index 2
}

TEST_F(HistoryManagerTest, GetHistoryWithoutPlaylistRef) {
    auto f1 = std::make_shared<Model::MediaFile>("1.mp3", "/p/1.mp3");
    historyManager->pushHistory(f1);
    
    // No playlist ref set
    auto indices = historyManager->getHistory();
    EXPECT_TRUE(indices.empty());
}
