/**
 * @file testPlaybackControllerCoverage.cpp
 * @brief Coverage tests for PlaybackControllerImpl
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/appcontroller/PlaybackController.h"
#include "controller/appcontroller/PlaylistManager.h"
#include "controller/appcontroller/HistoryManager.h"
#include "controller/MockAudioPlayer.h"
#include "model/MockPlayerState.h"
#include "model/MediaFile.h"

using namespace Model;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::StrictMock;
using ::testing::Invoke;

namespace Controller {

class PlaybackControllerCoverageTest : public ::testing::Test {
protected:
    std::shared_ptr<NiceMock<MockAudioPlayer>> mockAudioPlayer;
    std::shared_ptr<NiceMock<MockPlayerState>> mockPlayerState;
    std::shared_ptr<PlaylistManagerImpl> playlistManager;
    std::shared_ptr<HistoryManagerImpl> historyManager;
    std::shared_ptr<PlaybackControllerImpl> playbackController;

    void SetUp() override {
        mockAudioPlayer = std::make_shared<NiceMock<MockAudioPlayer>>();
        mockPlayerState = std::make_shared<NiceMock<MockPlayerState>>();
        playlistManager = std::make_shared<PlaylistManagerImpl>();
        historyManager = std::make_shared<HistoryManagerImpl>();
        
        playbackController = std::make_shared<PlaybackControllerImpl>(
            mockAudioPlayer,
            mockPlayerState,
            playlistManager.get(),
            historyManager.get()
        );
    }
    
    void addTrack(const std::string& path) {
        auto file = std::make_shared<Model::MediaFile>("song.mp3", path);
        playlistManager->getPlaylistRef().push_back(file);
    }
    
    // Accessors for private members via Friend mechanism
    void setOriginalOrder(const std::vector<std::shared_ptr<Model::MediaFile>>& order) {
        playbackController->mOriginalOrder = order;
    }
    
    std::vector<std::shared_ptr<Model::MediaFile>>& getOriginalOrder() {
        return playbackController->mOriginalOrder;
    }
    
    void setCurrentLoadedPath(const std::string& path) {
        playbackController->mCurrentLoadedPath = path;
    }
    
    std::string getCurrentLoadedPath() const {
        return playbackController->mCurrentLoadedPath;
    }
    
    void clearPlaylist() {
        playlistManager->clearPlaylist();
    }
    
    // Helper to set iterator
    void setIterator(int index) {
        auto& pl = playlistManager->getPlaylistRef();
        auto it = pl.begin();
        std::advance(it, index);
        playbackController->mCurrentTrackIterator = it;
    }
    
    void setIteratorEnd() {
        playbackController->mCurrentTrackIterator = playlistManager->getPlaylistRef().end();
    }
};

TEST_F(PlaybackControllerCoverageTest, NullComponents) {
    auto pc = std::make_shared<PlaybackControllerImpl>(nullptr, nullptr, nullptr, nullptr);
    EXPECT_FALSE(pc->loadTrack("path"));
    pc->play();
    pc->next(); 
    pc->previous(); // Should return safely
    pc->stop();
    pc->pause();
    pc->queueNext("path");
    pc->replaceQueue({});
    pc->playLibrary(0);
    EXPECT_EQ(pc->getCurrentTrackIndex(), -1);
}

TEST_F(PlaybackControllerCoverageTest, PlayUnmutesIfMuted) {
    addTrack("/path/1.mp3");
    setIterator(0);
    setCurrentLoadedPath("/path/1.mp3"); // Pretend loaded
    
    EXPECT_CALL(*mockPlayerState, isMuted()).WillOnce(Return(true));
    EXPECT_CALL(*mockPlayerState, setMuted(false)).Times(1);
    EXPECT_CALL(*mockPlayerState, getVolume()).WillOnce(Return(50));
    EXPECT_CALL(*mockAudioPlayer, setVolume(50)).Times(1);
    EXPECT_CALL(*mockAudioPlayer, play()).Times(1);
    
    EXPECT_CALL(*mockAudioPlayer, isLoaded()).WillRepeatedly(Return(true));
    
    playbackController->play();
}

TEST_F(PlaybackControllerCoverageTest, PlayReloadsIfPathChanged) {
    addTrack("/path/new.mp3");
    setIterator(0);
    setCurrentLoadedPath("/path/old.mp3"); // Mismatch
    
    EXPECT_CALL(*mockAudioPlayer, isLoaded()).WillRepeatedly(Return(true));
    // Expect loadTrack call
    EXPECT_CALL(*mockAudioPlayer, load("/path/new.mp3")).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioPlayer, play()).Times(1);
    
    playbackController->play();
}

TEST_F(PlaybackControllerCoverageTest, NextRepeatOne) {
    addTrack("/path/1.mp3");
    setIterator(0);
    
    EXPECT_CALL(*mockPlayerState, getRepeatMode()).WillOnce(Return(Model::RepeatMode::ONE));
    EXPECT_CALL(*mockAudioPlayer, seek(0)).Times(1);
    EXPECT_CALL(*mockAudioPlayer, play()).Times(1);
    EXPECT_CALL(*mockPlayerState, setCurrentPosition(0)).Times(1);
    EXPECT_CALL(*mockPlayerState, incrementPlaybackVersion()).Times(1);
    
    playbackController->next();
}

TEST_F(PlaybackControllerCoverageTest, ToggleShuffleLogic) {
    // 1. Shuffle ON
    addTrack("/1.mp3");
    addTrack("/2.mp3"); 
    setIterator(0); 
    
    EXPECT_CALL(*mockPlayerState, isShuffleEnabled()).WillOnce(Return(false));
    EXPECT_CALL(*mockPlayerState, setShuffleEnabled(true)).Times(1);
    
    playbackController->toggleShuffle();
    
    // originalOrder should contain 2.mp3
    auto& orig = getOriginalOrder();
    ASSERT_EQ(orig.size(), 1u);
    EXPECT_EQ(orig[0]->getPath(), "/2.mp3");
    
    // 2. Shuffle OFF
    // Assume state: current is 1.mp3. Playlist has [1, 2] (shuffled result, same here).
    // Original order has [2].
    
    EXPECT_CALL(*mockPlayerState, isShuffleEnabled()).WillOnce(Return(true));
    EXPECT_CALL(*mockPlayerState, setShuffleEnabled(false)).Times(1);
    
    playbackController->toggleShuffle();
    
    ASSERT_TRUE(getOriginalOrder().empty());
    // Playlist restored (cleared and rebuilt from current + original)
    // Should be [1, 2]
    auto& pl = playlistManager->getPlaylistRef();
    ASSERT_EQ(pl.size(), 2u);
}

TEST_F(PlaybackControllerCoverageTest, PlayLibraryEmpty) {
    // Empty library
    EXPECT_CALL(*mockAudioPlayer, stop()).Times(1);
    playbackController->playLibrary(0);
    EXPECT_TRUE(playlistManager->getPlaylistRef().empty());
}

TEST_F(PlaybackControllerCoverageTest, QueueNextAutoPlaysIfEmpty) {
    // Empty playlist
    EXPECT_TRUE(playlistManager->getPlaylistRef().empty());
    
    // queueNext should load and play
    EXPECT_CALL(*mockAudioPlayer, load("/path/1.mp3")).WillOnce(Return(true));
    EXPECT_CALL(*mockAudioPlayer, play()).Times(1);
    EXPECT_CALL(*mockPlayerState, setPlaybackStatus(Model::PlaybackStatus::PLAYING)).Times(1);
    EXPECT_CALL(*mockPlayerState, setCurrentTrackIndex(0)).Times(1);
    
    // create fake file for acquireMediaFile
    // PlaylistManagerImpl::acquireMediaFile creates new file if not found.
    // However, it creates with implementation logic.
    // Our logic is: queueNext calls acquireMediaFile.
    
    playbackController->queueNext("/path/1.mp3");
    
    EXPECT_EQ(playlistManager->getPlaylistSize(), 1u);
}

TEST_F(PlaybackControllerCoverageTest, QueueNextAppendsIfNotEmpty) {
    addTrack("/path/1.mp3");
    setIterator(0);
    
    // queueNext should append after 1.mp3
    EXPECT_CALL(*mockAudioPlayer, play()).Times(0);
    
    playbackController->queueNext("/path/2.mp3");
    
    auto& pl = playlistManager->getPlaylistRef();
    EXPECT_EQ(pl.size(), 2u);
    // 2nd item should be 2.mp3
    auto it = pl.begin();
    std::advance(it, 1);
    EXPECT_EQ((*it)->getPath(), "/path/2.mp3");
}

TEST_F(PlaybackControllerCoverageTest, ToggleRepeat) {
    // NONE -> ONE
    EXPECT_CALL(*mockPlayerState, getRepeatMode()).WillOnce(Return(Model::RepeatMode::NONE));
    EXPECT_CALL(*mockPlayerState, setRepeatMode(Model::RepeatMode::ONE)).Times(1);
    playbackController->toggleRepeat();
    
    // ONE -> NONE (assuming ANY other value goes to NONE based on code)
    EXPECT_CALL(*mockPlayerState, getRepeatMode()).WillOnce(Return(Model::RepeatMode::ONE));
    EXPECT_CALL(*mockPlayerState, setRepeatMode(Model::RepeatMode::NONE)).Times(1);
    playbackController->toggleRepeat();
}

TEST_F(PlaybackControllerCoverageTest, NextWithShuffleRemovesFromOriginal) {
    addTrack("/1.mp3");
    addTrack("/2.mp3");
    
    playbackController->playTrack(0); // Sets iterator to /1.mp3
    
    EXPECT_CALL(*mockPlayerState, isShuffleEnabled()).WillRepeatedly(Return(true));
    
    // Setup original order containing /1.mp3
    auto file1 = std::make_shared<Model::MediaFile>("1.mp3", "/1.mp3");
    std::vector<std::shared_ptr<Model::MediaFile>> original = { file1 };
    setOriginalOrder(original);
    
    // Trigger next
    EXPECT_CALL(*mockAudioPlayer, isLoaded()).WillRepeatedly(Return(false));
    EXPECT_CALL(*mockAudioPlayer, load(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockAudioPlayer, play()).WillRepeatedly(Return());
    
    playbackController->next();
    
    // Verify file1 removed from original order
    EXPECT_TRUE(getOriginalOrder().empty());
}

} // namespace Controller
