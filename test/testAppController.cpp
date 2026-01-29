/**
 * @file testAppController.cpp
 * @brief Unit Tests for AppController class.
 * @details Uses mock implementations for isolated testing.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include "../src/controller/AppController.h"
#include "../src/controller/IAudioPlayer.h"
#include "../src/controller/ISerialManager.h"
#include "../src/model/PlayerState.h"

using namespace Controller;
using namespace Model;

// ============================================================================
// Mock AudioPlayer
// ============================================================================

class MockAudioPlayer : public IAudioPlayer {
public:
    bool initializeCalled = false;
    bool shutdownCalled = false;
    std::string loadedFile;
    bool playingState = false;
    bool pausedState = false;
    int volumeLevel = 50;
    AudioState currentState = AudioState::IDLE;
    AudioCallback callback;

    bool initialize() override {
        initializeCalled = true;
        currentState = AudioState::IDLE;
        return true;
    }

    void shutdown() override {
        shutdownCalled = true;
        currentState = AudioState::IDLE;
    }

    bool load(const std::string& filePath) override {
        loadedFile = filePath;
        currentState = AudioState::LOADED;
        if (callback) callback(currentState, 0);
        return true;
    }

    void unload() override {
        loadedFile.clear();
        currentState = AudioState::IDLE;
    }

    void play() override {
        playingState = true;
        pausedState = false;
        currentState = AudioState::PLAYING;
        if (callback) callback(currentState, 0);
    }

    void pause() override {
        playingState = false;
        pausedState = true;
        currentState = AudioState::PAUSED;
        if (callback) callback(currentState, 0);
    }

    void stop() override {
        playingState = false;
        pausedState = false;
        currentState = AudioState::LOADED;
        if (callback) callback(currentState, 0);
    }

    void seek(uint32_t) override {}

    void setVolume(int vol) override {
        volumeLevel = vol;
    }

    int getVolume() const override {
        return volumeLevel;
    }

    AudioState getState() const override {
        return currentState;
    }

    uint32_t getPosition() const override { return 0; }
    uint32_t getDuration() const override { return 0; }

    bool isLoaded() const override {
        return currentState != AudioState::IDLE;
    }

    bool isPlaying() const override {
        return playingState;
    }

    void setCallback(AudioCallback cb) override {
        callback = cb;
    }
};

// ============================================================================
// Mock SerialManager
// ============================================================================

class MockSerialManager : public ISerialManager {
public:
    bool connected = false;
    std::string connectedPort;
    uint32_t connectedBaud = 0;
    std::vector<std::string> sentMessages;
    SerialDataCallback dataCallback;
    SerialStateCallback stateCallback;
    SerialState currentState = SerialState::DISCONNECTED;

    bool connect(const std::string& portName, uint32_t baudRate) override {
        connected = true;
        connectedPort = portName;
        connectedBaud = baudRate;
        currentState = SerialState::CONNECTED;
        if (stateCallback) stateCallback(currentState);
        return true;
    }

    void disconnect() override {
        connected = false;
        connectedPort.clear();
        currentState = SerialState::DISCONNECTED;
        if (stateCallback) stateCallback(currentState);
    }

    bool isConnected() const override {
        return connected;
    }

    SerialState getState() const override {
        return currentState;
    }

    int send(const std::string& data) override {
        sentMessages.push_back(data);
        return static_cast<int>(data.length());
    }

    int sendBytes(const uint8_t*, size_t length) override {
        return static_cast<int>(length);
    }

    int read(uint8_t*, size_t) override { return 0; }
    std::string readLine(uint32_t) override { return ""; }
    size_t available() const override { return 0; }

    void setDataCallback(SerialDataCallback cb) override {
        dataCallback = cb;
    }

    void setStateCallback(SerialStateCallback cb) override {
        stateCallback = cb;
    }

    std::string getPortName() const override { return connectedPort; }
    uint32_t getBaudRate() const override { return connectedBaud; }
    void flush() override {}

    // Test helper: simulate receiving data
    void simulateReceive(const std::string& data) {
        if (dataCallback) {
            dataCallback(data);
        }
    }
};

// ============================================================================
// Test Fixture
// ============================================================================

class AppControllerTest : public ::testing::Test {
protected:
    std::shared_ptr<MockAudioPlayer> mockAudio;
    std::shared_ptr<MockSerialManager> mockSerial;
    std::shared_ptr<PlayerState> playerState;
    std::unique_ptr<AppController> controller;

    void SetUp() override {
        mockAudio = std::make_shared<MockAudioPlayer>();
        mockSerial = std::make_shared<MockSerialManager>();
        playerState = std::make_shared<PlayerState>();

        controller = std::make_unique<AppController>(
            mockAudio, mockSerial, playerState
        );
    }

    void TearDown() override {
        controller.reset();
    }
};

// ============================================================================
// Lifecycle Tests
// ============================================================================

TEST_F(AppControllerTest, Construction) {
    EXPECT_EQ(controller->getState(), AppState::UNINITIALIZED);
}

TEST_F(AppControllerTest, Initialize) {
    EXPECT_TRUE(controller->initialize());
    EXPECT_EQ(controller->getState(), AppState::READY);
    EXPECT_TRUE(mockAudio->initializeCalled);
}

TEST_F(AppControllerTest, DoubleInitialize) {
    EXPECT_TRUE(controller->initialize());
    EXPECT_TRUE(controller->initialize());  // Should succeed
    EXPECT_EQ(controller->getState(), AppState::READY);
}

TEST_F(AppControllerTest, Shutdown) {
    controller->initialize();
    controller->shutdown();
    
    EXPECT_TRUE(mockAudio->shutdownCalled);
    EXPECT_EQ(controller->getState(), AppState::UNINITIALIZED);
}

// ============================================================================
// Serial Connection Tests
// ============================================================================

TEST_F(AppControllerTest, ConnectToBoard) {
    controller->initialize();
    
    EXPECT_TRUE(controller->connectToBoard("/dev/ttyUSB0", 115200));
    EXPECT_TRUE(controller->isConnectedToBoard());
    EXPECT_EQ(mockSerial->connectedPort, "/dev/ttyUSB0");
    EXPECT_EQ(mockSerial->connectedBaud, 115200u);
}

TEST_F(AppControllerTest, DisconnectFromBoard) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    controller->disconnectFromBoard();
    
    EXPECT_FALSE(controller->isConnectedToBoard());
}

TEST_F(AppControllerTest, StateChangesToRunningOnConnect) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    
    EXPECT_EQ(controller->getState(), AppState::RUNNING);
}

// ============================================================================
// Playback Control Tests
// ============================================================================

TEST_F(AppControllerTest, LoadTrack) {
    controller->initialize();
    
    EXPECT_TRUE(controller->loadTrack("/path/to/song.mp3"));
    EXPECT_EQ(mockAudio->loadedFile, "/path/to/song.mp3");
}

TEST_F(AppControllerTest, Play) {
    controller->initialize();
    controller->loadTrack("/path/to/song.mp3");
    controller->play();
    
    EXPECT_TRUE(mockAudio->playingState);
    EXPECT_EQ(playerState->getPlaybackState(), PlaybackState::PLAYING);
}

TEST_F(AppControllerTest, Pause) {
    controller->initialize();
    controller->loadTrack("/path/to/song.mp3");
    controller->play();
    controller->pause();
    
    EXPECT_TRUE(mockAudio->pausedState);
    EXPECT_EQ(playerState->getPlaybackState(), PlaybackState::PAUSED);
}

TEST_F(AppControllerTest, Stop) {
    controller->initialize();
    controller->loadTrack("/path/to/song.mp3");
    controller->play();
    controller->stop();
    
    EXPECT_FALSE(mockAudio->playingState);
    EXPECT_EQ(playerState->getPlaybackState(), PlaybackState::STOPPED);
}

// ============================================================================
// Volume Control Tests
// ============================================================================

TEST_F(AppControllerTest, SetVolume) {
    controller->initialize();
    
    controller->setVolume(75);
    EXPECT_EQ(controller->getVolume(), 75);
    EXPECT_EQ(playerState->getVolume(), 75);
    EXPECT_EQ(mockAudio->volumeLevel, 75);
}

TEST_F(AppControllerTest, VolumeClamp) {
    controller->initialize();
    
    controller->setVolume(150);
    EXPECT_EQ(controller->getVolume(), 100);
    
    controller->setVolume(-50);
    EXPECT_EQ(controller->getVolume(), 0);
}

TEST_F(AppControllerTest, ToggleMute) {
    controller->initialize();
    controller->setVolume(80);
    
    // Mute
    controller->toggleMute();
    EXPECT_TRUE(playerState->isMuted());
    EXPECT_EQ(mockAudio->volumeLevel, 0);
    
    // Unmute
    controller->toggleMute();
    EXPECT_FALSE(playerState->isMuted());
    EXPECT_EQ(mockAudio->volumeLevel, 80);
}

// ============================================================================
// Playlist Tests
// ============================================================================

TEST_F(AppControllerTest, AddToPlaylist) {
    controller->initialize();
    
    controller->addToPlaylist("/path/song1.mp3");
    controller->addToPlaylist("/path/song2.mp3");
    
    EXPECT_EQ(controller->getPlaylistSize(), 2u);
}

TEST_F(AppControllerTest, ClearPlaylist) {
    controller->initialize();
    
    controller->addToPlaylist("/path/song1.mp3");
    controller->addToPlaylist("/path/song2.mp3");
    controller->clearPlaylist();
    
    EXPECT_EQ(controller->getPlaylistSize(), 0u);
}

TEST_F(AppControllerTest, FirstTrackSetsIndex) {
    controller->initialize();
    
    EXPECT_EQ(playerState->getCurrentTrackIndex(), -1);
    
    controller->addToPlaylist("/path/song1.mp3");
    EXPECT_EQ(playerState->getCurrentTrackIndex(), 0);
}

// ============================================================================
// Command Processing Tests (via Mock Serial)
// ============================================================================

TEST_F(AppControllerTest, CommandPlay) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    controller->loadTrack("/path/song.mp3");
    
    mockSerial->simulateReceive("PLAY");
    EXPECT_TRUE(mockAudio->playingState);
}

TEST_F(AppControllerTest, CommandPause) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    controller->loadTrack("/path/song.mp3");
    controller->play();
    
    mockSerial->simulateReceive("PAUSE");
    EXPECT_TRUE(mockAudio->pausedState);
}

TEST_F(AppControllerTest, CommandVolume) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    
    mockSerial->simulateReceive("VOL:65");
    EXPECT_EQ(playerState->getVolume(), 65);
}

TEST_F(AppControllerTest, CommandMute) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    controller->setVolume(80);
    
    mockSerial->simulateReceive("MUTE");
    EXPECT_TRUE(playerState->isMuted());
}

TEST_F(AppControllerTest, CommandCaseInsensitive) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    controller->loadTrack("/path/song.mp3");
    
    mockSerial->simulateReceive("play");  // lowercase
    EXPECT_TRUE(mockAudio->playingState);
}

// ============================================================================
// Status Reporting Tests
// ============================================================================

TEST_F(AppControllerTest, StatusSentOnConnect) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    
    // Should have sent status after connect
    EXPECT_FALSE(mockSerial->sentMessages.empty());
    EXPECT_TRUE(mockSerial->sentMessages.back().find("STATUS:") == 0);
}

TEST_F(AppControllerTest, StatusContainsPlaybackState) {
    controller->initialize();
    controller->connectToBoard("/dev/ttyUSB0");
    controller->play();
    
    bool foundPlaying = false;
    for (const auto& msg : mockSerial->sentMessages) {
        if (msg.find("PLAYING") != std::string::npos) {
            foundPlaying = true;
            break;
        }
    }
    EXPECT_TRUE(foundPlaying);
}

// ============================================================================
// State Callback Tests
// ============================================================================

TEST_F(AppControllerTest, StateCallback) {
    int callCount = 0;
    AppState lastState = AppState::UNINITIALIZED;
    
    controller->setStateCallback([&callCount, &lastState](AppState state) {
        callCount++;
        lastState = state;
    });
    
    controller->initialize();
    
    EXPECT_GE(callCount, 1);
    EXPECT_EQ(lastState, AppState::READY);
}
