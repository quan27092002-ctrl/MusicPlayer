/**
 * @file testBoardCommunicator.cpp
 * @brief Unit Tests for BoardCommunicatorImpl class.
 * @details Tests connection, status updates, and command parsing logic.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/appcontroller/BoardCommunicator.h"
#include "controller/MockSerialManager.h"
#include "model/MockPlayerState.h"

using namespace Controller;
using namespace Model;
using ::testing::_;
using ::testing::Return;
using ::testing::NiceMock;
using ::testing::Throw;
using ::testing::Invoke;

class BoardCommunicatorTest : public ::testing::Test {
protected:
    std::shared_ptr<NiceMock<MockSerialManager>> mockSerial;
    std::shared_ptr<NiceMock<MockPlayerState>> mockPlayerState;
    std::shared_ptr<BoardCommunicatorImpl> boardComm;
    
    // Callback verification variables
    BoardEvent lastEvent = static_cast<BoardEvent>(-1);
    int lastValue = -1;
    bool eventReceived = false;

    void SetUp() override {
        mockSerial = std::make_shared<NiceMock<MockSerialManager>>();
        mockPlayerState = std::make_shared<NiceMock<MockPlayerState>>();
        boardComm = std::make_shared<BoardCommunicatorImpl>(mockSerial, mockPlayerState);
        
        // Default callback
        boardComm->setBoardEventCallback([this](BoardEvent event, int value) {
            lastEvent = event;
            lastValue = value;
            eventReceived = true;
        });
    }
};

// ============================================================================
// Connection Tests
// ============================================================================

TEST_F(BoardCommunicatorTest, ConnectSuccess) {
    EXPECT_CALL(*mockSerial, connect("/dev/ttyUSB0", 115200))
        .WillOnce(Return(true));
        
    EXPECT_TRUE(boardComm->connectToBoard("/dev/ttyUSB0", 115200));
}

TEST_F(BoardCommunicatorTest, ConnectFail) {
    EXPECT_CALL(*mockSerial, connect(_, _))
        .WillOnce(Return(false));
        
    EXPECT_FALSE(boardComm->connectToBoard("/dev/ttyUSB0", 115200));
}

TEST_F(BoardCommunicatorTest, ConnectWithNullSerialManager) {
    auto bc = std::make_shared<BoardCommunicatorImpl>(nullptr, mockPlayerState);
    EXPECT_FALSE(bc->connectToBoard("/dev/ttyUSB0", 115200));
}

TEST_F(BoardCommunicatorTest, Disconnect) {
    EXPECT_CALL(*mockSerial, disconnect()).Times(1);
    boardComm->disconnectFromBoard();
}

TEST_F(BoardCommunicatorTest, DisconnectWithNullSerialManager) {
    auto bc = std::make_shared<BoardCommunicatorImpl>(nullptr, mockPlayerState);
    // Should not crash
    bc->disconnectFromBoard();
}

TEST_F(BoardCommunicatorTest, IsConnectedCheck) {
    EXPECT_CALL(*mockSerial, isConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(false));
        
    EXPECT_TRUE(boardComm->isConnectedToBoard());
    EXPECT_FALSE(boardComm->isConnectedToBoard());
}

TEST_F(BoardCommunicatorTest, IsConnectedNullSerial) {
    auto bc = std::make_shared<BoardCommunicatorImpl>(nullptr, mockPlayerState);
    EXPECT_FALSE(bc->isConnectedToBoard());
}

TEST_F(BoardCommunicatorTest, GetAvailablePortsDelegation) {
    std::vector<std::string> ports = {"COM1", "COM2"};
    EXPECT_CALL(*mockSerial, getAvailablePorts())
        .WillOnce(Return(ports));
        
    auto result = boardComm->getAvailablePorts();
    EXPECT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "COM1");
}

TEST_F(BoardCommunicatorTest, GetAvailablePortsNullSerial) {
    auto bc = std::make_shared<BoardCommunicatorImpl>(nullptr, mockPlayerState);
    auto result = bc->getAvailablePorts();
    EXPECT_TRUE(result.empty());
}

// ============================================================================
// Status Sending Tests
// ============================================================================

TEST_F(BoardCommunicatorTest, SendStatusNotConnected) {
    EXPECT_CALL(*mockSerial, isConnected()).WillOnce(Return(false));
    EXPECT_CALL(*mockSerial, send(_)).Times(0);
    
    boardComm->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusNullSerial) {
    auto bc = std::make_shared<BoardCommunicatorImpl>(nullptr, mockPlayerState);
    // Should verify send is NOT called (implicit since null) and no crash
    bc->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusPlaying) {
    EXPECT_CALL(*mockSerial, isConnected()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockPlayerState, getPlaybackStatus()).WillRepeatedly(Return(Model::PlaybackStatus::PLAYING));
    EXPECT_CALL(*mockPlayerState, getVolume()).WillRepeatedly(Return(50));
    EXPECT_CALL(*mockPlayerState, isMuted()).WillRepeatedly(Return(false));
    
    // Expect output format
    EXPECT_CALL(*mockSerial, send(::testing::HasSubstr("STATUS:PLAYING,VOL:50,MUTE:0")));
    
    boardComm->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusPausedMuted) {
    EXPECT_CALL(*mockSerial, isConnected()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockPlayerState, getPlaybackStatus()).WillRepeatedly(Return(Model::PlaybackStatus::PAUSED));
    EXPECT_CALL(*mockPlayerState, getVolume()).WillRepeatedly(Return(25));
    EXPECT_CALL(*mockPlayerState, isMuted()).WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockSerial, send(::testing::HasSubstr("STATUS:PAUSED,VOL:25,MUTE:1")));
    
    boardComm->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusStopped) {
    EXPECT_CALL(*mockSerial, isConnected()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockPlayerState, getPlaybackStatus()).WillRepeatedly(Return(Model::PlaybackStatus::STOPPED));
    
    EXPECT_CALL(*mockSerial, send(::testing::HasSubstr("STATUS:STOPPED")));
    
    boardComm->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusNullPlayerState) {
    auto bc = std::make_shared<BoardCommunicatorImpl>(mockSerial, nullptr);
    EXPECT_CALL(*mockSerial, isConnected()).WillRepeatedly(Return(true));
    
    EXPECT_CALL(*mockSerial, send(::testing::HasSubstr("STATUS:UNKNOWN")));
    
    bc->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusWithTrackIndex) {
    EXPECT_CALL(*mockSerial, isConnected()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mockPlayerState, getPlaybackStatus()).WillRepeatedly(Return(Model::PlaybackStatus::PLAYING));
    
    boardComm->setCurrentTrackIndexGetter([]() { return 5; });
    
    EXPECT_CALL(*mockSerial, send(::testing::HasSubstr(",TRACK:5")));
    
    
    boardComm->sendStatusToBoard();
}

TEST_F(BoardCommunicatorTest, SendStatusUnknown) {
    EXPECT_CALL(*mockSerial, isConnected()).WillRepeatedly(Return(true));
    // Force an unknown status to test default switch case
    EXPECT_CALL(*mockPlayerState, getPlaybackStatus())
        .WillRepeatedly(Return(static_cast<Model::PlaybackStatus>(99)));
    
    // Should still send VOL and MUTE but no PLAYING/PAUSED/STOPPED
    EXPECT_CALL(*mockSerial, send(::testing::HasSubstr("STATUS:,VOL:")));
    
    boardComm->sendStatusToBoard();
}

// ============================================================================
// Command Parsing Tests
// ============================================================================

TEST_F(BoardCommunicatorTest, ParseRVNormal) {
    // RV:2048 (approx 50%)
    // 2048 * 100 / 4095 = 50
    boardComm->processCommand("RV:2048");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::SET_VOLUME);
    EXPECT_EQ(lastValue, 50);
}

TEST_F(BoardCommunicatorTest, ParseVRNormal) {
    // Alternate format VR:
    boardComm->processCommand("VR:4095");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::SET_VOLUME);
    EXPECT_EQ(lastValue, 100);
}

TEST_F(BoardCommunicatorTest, ParseRVLow) {
    // Check clamping logic for adc < 0
    boardComm->processCommand("RV:-100");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastValue, 0);
}

TEST_F(BoardCommunicatorTest, ParseRVHigh) {
    // Check clamping logic for adc > 4095
    boardComm->processCommand("RV:5000");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastValue, 100);
}

TEST_F(BoardCommunicatorTest, ParseRVInvalidFormat) {
    // Should catch exception and not crash
    // Also should not trigger callback
    eventReceived = false;
    boardComm->processCommand("RV:NotANumber");
    
    EXPECT_FALSE(eventReceived);
}

TEST_F(BoardCommunicatorTest, ParseCmdPlay) {
    boardComm->processCommand("CMD:PLAY");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::PLAY);
}

TEST_F(BoardCommunicatorTest, ParseCmdPauseLowercase) {
    boardComm->processCommand("cmd:pause");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::PAUSE);
}

TEST_F(BoardCommunicatorTest, ParseCmdStop) {
    boardComm->processCommand("CMD:STOP");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::STOP);
}

TEST_F(BoardCommunicatorTest, ParseCmdNext) {
    boardComm->processCommand("CMD:NEXT");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::NEXT);
}

TEST_F(BoardCommunicatorTest, ParseCmdPrev) {
    boardComm->processCommand("CMD:PREV");
    
    ASSERT_TRUE(eventReceived);
    EXPECT_EQ(lastEvent, BoardEvent::PREV);
}

TEST_F(BoardCommunicatorTest, ParserUnknownException) {
    // To test catch(...) block, simulate callback throwing unknwon exception
    // when processing RV
    boardComm->setBoardEventCallback([](BoardEvent, int) {
        throw 42; // Throw an int, not std::exception
    });

    // Verify it doesn't crash
    boardComm->processCommand("RV:2000");
}

TEST_F(BoardCommunicatorTest, NullCallback) {
    boardComm->setBoardEventCallback(nullptr);
    // Should return early and safely
    boardComm->processCommand("CMD:PLAY");
}

TEST_F(BoardCommunicatorTest, ParseInvalidCommand) {
    // Send a command that matches neither RV nor CMD prefixes
    boardComm->processCommand("HELLO:WORLD");
    
    // Should just return without triggering any event
    EXPECT_FALSE(eventReceived);
}
