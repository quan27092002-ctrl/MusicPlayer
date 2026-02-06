#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include "../src/controller/AppController.h"
#include "../src/controller/IAudioPlayer.h"
#include "../src/controller/ISerialManager.h"
#include "../src/model/PlayerState.h"

using namespace Controller;
using namespace Model;

// Minimal Mocks needed for PlaybackController testing

class MockAudioPlayerPlayback : public IAudioPlayer {
public:
    AudioState currentState = AudioState::IDLE;
    std::string loadedFile;
    
    bool initialize() override { return true; }
    void shutdown() override {}
    bool load(const std::string& filePath) override {
        loadedFile = filePath;
        currentState = AudioState::LOADED;
        return true; 
    }
    void unload() override { currentState = AudioState::IDLE; }
    void play() override { currentState = AudioState::PLAYING; }
    void pause() override { currentState = AudioState::PAUSED; }
    void stop() override { currentState = AudioState::LOADED; } // Or STOPPED
    void seek(uint32_t) override {}
    void setVolume(int) override {}
    int getVolume() const override { return 50; }
    AudioState getState() const override { return currentState; }
    uint32_t getPosition() const override { return 0; }
    uint32_t getDuration() const override { return 100; }
    bool isLoaded() const override { return currentState != AudioState::IDLE; }
    bool isPlaying() const override { return currentState == AudioState::PLAYING; }
    void setCallback(AudioCallback) override {}
    void setFinishedCallback(std::function<void()>) override {}
};

class MockSerialManagerPlayback : public ISerialManager {
public:
    bool connect(const std::string&, uint32_t) override { return true; }
    void disconnect() override {}
    bool isConnected() const override { return true; }
    SerialState getState() const override { return SerialState::CONNECTED; }
    int send(const std::string&) override { return 0; }
    int sendBytes(const uint8_t*, size_t) override { return 0; }
    int read(uint8_t*, size_t) override { return 0; }
    std::string readLine(uint32_t) override { return ""; }
    size_t available() const override { return 0; }
    void setDataCallback(SerialDataCallback) override {}
    void setStateCallback(SerialStateCallback) override {}
    std::string getPortName() const override { return ""; }
    uint32_t getBaudRate() const override { return 0; }
    void flush() override {}
};

class PlaybackFixTest : public ::testing::Test {
protected:
    std::shared_ptr<MockAudioPlayerPlayback> mockAudio;
    std::shared_ptr<MockSerialManagerPlayback> mockSerial;
    std::shared_ptr<PlayerState> playerState;
    std::unique_ptr<AppController> controller;

    void SetUp() override {
        mockAudio = std::make_shared<MockAudioPlayerPlayback>();
        mockSerial = std::make_shared<MockSerialManagerPlayback>();
        playerState = std::make_shared<PlayerState>();

        controller = std::make_unique<AppController>(
            mockAudio, mockSerial, playerState
        );
        controller->initialize();
    }
};

TEST_F(PlaybackFixTest, QueueClearsAfterLastSong) {
    // 1. Add 2 songs
    controller->addToPlaylist("/song1.mp3");
    controller->addToPlaylist("/song2.mp3");
    
    // 2. Play first song
    controller->playTrack(0);
    EXPECT_EQ(playerState->getCurrentTrackIndex(), 0);
    
    // 3. Move to next (Song 1 done)
    controller->next();
    // Expect: Playing Song 2. Song 1 in history.
    EXPECT_EQ(playerState->getCurrentTrackIndex(), 0); 
    EXPECT_EQ(mockAudio->loadedFile, "/song2.mp3");
    
    // 4. Move to next (Song 2 done - LAST SONG)
    controller->next();
    
    // Expect: 
    // - Queue is empty (or iterator at end)
    // - Current Index is -1
    // - Playback Stopped
    // - History has 2 items
    
    auto historySize = controller->getHistorySize();
    ASSERT_EQ(historySize, 2u);
    
    // Check Current Track Index -> Should be -1 (Nothing playing)
    int idx = playerState->getCurrentTrackIndex();
    EXPECT_EQ(idx, -1); 
    
    // Verify Playback Status is STOPPED
    EXPECT_EQ(playerState->getPlaybackStatus(), PlaybackStatus::STOPPED);
}
