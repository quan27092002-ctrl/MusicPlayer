#include "gtest/gtest.h"
#include "controller/AppController.h"
#include "controller/appcontroller/PlaylistManager.h"
#include "model/PlayerState.h"
#include "controller/IAudioPlayer.h"
#include "controller/ISerialManager.h"
#include <memory>
#include <filesystem>
#include <fstream>

using namespace Controller;
using namespace Model;

// Minimal Mocks
class MockAudioPlayer : public IAudioPlayer {
public:
    bool initialize() override { return true; }
    void shutdown() override {}
    bool load(const std::string&) override { return true; }
    void unload() override {}
    void play() override {}
    void pause() override {}
    void stop() override {}
    void seek(uint32_t) override {}
    void setVolume(int) override {}
    int getVolume() const override { return 50; }
    AudioState getState() const override { return AudioState::IDLE; }
    uint32_t getPosition() const override { return 0; }
    uint32_t getDuration() const override { return 0; }
    bool isLoaded() const override { return false; }
    // IAudioPlayback
    bool isPlaying() const override { return false; }
    void setFinishedCallback(std::function<void()>) override {}
    
    // IAudioLifecycle
    void setCallback(AudioCallback) override {}
};

class MockSerialManager : public ISerialManager {
public:
    // ISerialConnection
    bool connect(const std::string&, uint32_t) override { return true; }
    void disconnect() override {}
    bool isConnected() const override { return false; }
    SerialState getState() const override { return SerialState::DISCONNECTED; }
    std::string getPortName() const override { return ""; }
    uint32_t getBaudRate() const override { return 0; }
    void setStateCallback(SerialStateCallback) override {}
    std::vector<std::string> getAvailablePorts() const override { return {}; }

    // ISerialIO
    int send(const std::string&) override { return 0; }
    int sendBytes(const uint8_t*, size_t) override { return 0; }
    int read(uint8_t*, size_t) override { return 0; }
    std::string readLine(uint32_t) override { return ""; }
    size_t available() const override { return 0; }
    void flush() override {}
    void setDataCallback(SerialDataCallback) override {}
};

class QueuePopulationTest : public ::testing::Test {
protected:
    std::shared_ptr<AppController> appController;
    std::shared_ptr<MockAudioPlayer> audioPlayer;
    std::shared_ptr<MockSerialManager> serialManager;
    std::shared_ptr<PlayerState> playerState;
    std::string testDir = "test_music_queue";

    void SetUp() override {
        playerState = std::make_shared<PlayerState>();
        audioPlayer = std::make_shared<MockAudioPlayer>();
        serialManager = std::make_shared<MockSerialManager>();
        appController = std::make_shared<AppController>(audioPlayer, serialManager, playerState);
        appController->initialize();

        std::filesystem::create_directory(testDir);
        std::ofstream(testDir + "/song1.mp3").close();
        std::ofstream(testDir + "/song2.mp3").close();
    }

    void TearDown() override {
        std::filesystem::remove_all(testDir);
        appController->shutdown();
    }
};

TEST_F(QueuePopulationTest, LoadDirectoryDoesNotpopulateQueue) {
    // Action: Load directory
    size_t loaded = appController->loadDirectory(testDir);
    
    // Check Library Size
    EXPECT_EQ(loaded, 2);
    EXPECT_EQ(appController->getLibrarySize(), 2);
    
    // Check Queue Size - SHOULD BE 0
    EXPECT_EQ(appController->getPlaylistSize(), 0);
    
    // Check Current Track Index
    EXPECT_EQ(playerState->getCurrentTrackIndex(), -1);
}
