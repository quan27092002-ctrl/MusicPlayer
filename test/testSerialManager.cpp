/**
 * @file testSerialManager.cpp
 * @brief Unit Tests for SerialManager class.
 * @details Tests basic functionality. Hardware tests are skipped if no serial port available.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include "../src/controller/SerialManager.h"

using namespace Controller;

// ============================================================================
// Basic Tests (No hardware required)
// ============================================================================

TEST(SerialManagerBasicTest, Construction) {
    SerialManager serial;
    // Should not crash on construction
    SUCCEED();
}

TEST(SerialManagerBasicTest, InitialState) {
    SerialManager serial;
    
    EXPECT_EQ(serial.getState(), SerialState::DISCONNECTED);
    EXPECT_FALSE(serial.isConnected());
    EXPECT_EQ(serial.getPortName(), "");
    EXPECT_EQ(serial.getBaudRate(), 0u);
    EXPECT_EQ(serial.available(), 0u);
}

TEST(SerialManagerBasicTest, ConnectInvalidPort) {
    SerialManager serial;
    
    // Connecting to non-existent port should fail
    EXPECT_FALSE(serial.connect("/dev/nonexistent_port_xyz", 115200));
    EXPECT_EQ(serial.getState(), SerialState::ERROR);
    EXPECT_FALSE(serial.isConnected());
}

TEST(SerialManagerBasicTest, DisconnectWhenNotConnected) {
    SerialManager serial;
    
    // Should not crash when disconnecting without connection
    serial.disconnect();
    EXPECT_EQ(serial.getState(), SerialState::DISCONNECTED);
}

TEST(SerialManagerBasicTest, SendWithoutConnection) {
    SerialManager serial;
    
    // Should return -1 when not connected
    EXPECT_EQ(serial.send("test"), -1);
    
    uint8_t data[] = {0x01, 0x02, 0x03};
    EXPECT_EQ(serial.sendBytes(data, 3), -1);
}

TEST(SerialManagerBasicTest, ReadWithoutConnection) {
    SerialManager serial;
    
    uint8_t buffer[10];
    EXPECT_EQ(serial.read(buffer, 10), -1);
    EXPECT_EQ(serial.readLine(100), "");
}

TEST(SerialManagerBasicTest, FlushWithoutConnection) {
    SerialManager serial;
    
    // Should not crash
    serial.flush();
    SUCCEED();
}

TEST(SerialManagerBasicTest, CallbackSetting) {
    SerialManager serial;
    
    bool dataCalled = false;
    bool stateCalled = false;
    
    serial.setDataCallback([&](const std::string&) {
        dataCalled = true;
    });
    
    serial.setStateCallback([&](SerialState) {
        stateCalled = true;
    });
    
    // Callbacks are set but not called without actual events
    SUCCEED();
}

TEST(SerialManagerBasicTest, StateCallbackOnInvalidConnect) {
    SerialManager serial;
    
    std::atomic<int> callCount{0};
    SerialState lastState = SerialState::DISCONNECTED;
    
    serial.setStateCallback([&](SerialState state) {
        callCount++;
        lastState = state;
    });
    
    // Attempt to connect to invalid port
    serial.connect("/dev/nonexistent_xyz", 115200);
    
    // Should have received CONNECTING and ERROR callbacks
    EXPECT_GE(callCount.load(), 2);
    EXPECT_EQ(lastState, SerialState::ERROR);
}

// ============================================================================
// Tests with Virtual Serial Port (PTY)
// These tests create a pseudo-terminal for testing without real hardware
// ============================================================================

class SerialManagerPTYTest : public ::testing::Test {
protected:
    int masterFd = -1;
    int slaveFd = -1;
    std::string slaveName;
    bool ptyAvailable = false;

    void SetUp() override {
        // Try to create a pseudo-terminal pair
        masterFd = posix_openpt(O_RDWR | O_NOCTTY);
        if (masterFd < 0) {
            GTEST_SKIP() << "Cannot create PTY - skipping test";
            return;
        }

        if (grantpt(masterFd) != 0 || unlockpt(masterFd) != 0) {
            close(masterFd);
            masterFd = -1;
            GTEST_SKIP() << "Cannot configure PTY - skipping test";
            return;
        }

        char* name = ptsname(masterFd);
        if (name == nullptr) {
            close(masterFd);
            masterFd = -1;
            GTEST_SKIP() << "Cannot get PTY slave name - skipping test";
            return;
        }

        slaveName = std::string(name);
        ptyAvailable = true;
    }

    void TearDown() override {
        if (slaveFd >= 0) {
            close(slaveFd);
        }
        if (masterFd >= 0) {
            close(masterFd);
        }
    }
};

TEST_F(SerialManagerPTYTest, ConnectToVirtualPort) {
    if (!ptyAvailable) {
        GTEST_SKIP();
    }

    SerialManager serial;
    EXPECT_TRUE(serial.connect(slaveName, 9600));
    EXPECT_TRUE(serial.isConnected());
    EXPECT_EQ(serial.getState(), SerialState::CONNECTED);
    EXPECT_EQ(serial.getPortName(), slaveName);
    EXPECT_EQ(serial.getBaudRate(), 9600u);
    
    serial.disconnect();
    EXPECT_FALSE(serial.isConnected());
}

TEST_F(SerialManagerPTYTest, SendData) {
    if (!ptyAvailable) {
        GTEST_SKIP();
    }

    SerialManager serial;
    ASSERT_TRUE(serial.connect(slaveName, 115200));

    // Send data
    int sent = serial.send("Hello World\n");
    EXPECT_GT(sent, 0);

    // Read from master side
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    char buffer[64];
    int n = read(masterFd, buffer, sizeof(buffer) - 1);
    EXPECT_GT(n, 0);
    
    if (n > 0) {
        buffer[n] = '\0';
        EXPECT_STREQ(buffer, "Hello World\n");
    }

    serial.disconnect();
}

TEST_F(SerialManagerPTYTest, ReceiveCallback) {
    if (!ptyAvailable) {
        GTEST_SKIP();
    }

    SerialManager serial;
    
    std::atomic<bool> dataReceived{false};
    std::string receivedData;
    
    serial.setDataCallback([&](const std::string& data) {
        receivedData = data;
        dataReceived = true;
    });

    ASSERT_TRUE(serial.connect(slaveName, 115200));

    // Write to master side (simulating S32K sending data)
    const char* testMessage = "PLAY\n";
    write(masterFd, testMessage, strlen(testMessage));

    // Wait for callback
    auto start = std::chrono::steady_clock::now();
    while (!dataReceived.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::milliseconds(1000)) {
            break;  // Timeout
        }
    }

    EXPECT_TRUE(dataReceived.load());
    EXPECT_EQ(receivedData, "PLAY");

    serial.disconnect();
}

TEST_F(SerialManagerPTYTest, MultipleLines) {
    if (!ptyAvailable) {
        GTEST_SKIP();
    }

    SerialManager serial;
    
    std::atomic<int> lineCount{0};
    std::vector<std::string> lines;
    std::mutex linesMutex;
    
    serial.setDataCallback([&](const std::string& data) {
        std::lock_guard<std::mutex> lock(linesMutex);
        lines.push_back(data);
        lineCount++;
    });

    ASSERT_TRUE(serial.connect(slaveName, 115200));

    // Send multiple lines from master
    write(masterFd, "LINE1\n", 6);
    write(masterFd, "LINE2\n", 6);
    write(masterFd, "LINE3\n", 6);

    // Wait for all lines
    auto start = std::chrono::steady_clock::now();
    while (lineCount.load() < 3) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::milliseconds(1000)) {
            break;
        }
    }

    EXPECT_EQ(lineCount.load(), 3);
    
    {
        std::lock_guard<std::mutex> lock(linesMutex);
        ASSERT_EQ(lines.size(), 3u);
        EXPECT_EQ(lines[0], "LINE1");
        EXPECT_EQ(lines[1], "LINE2");
        EXPECT_EQ(lines[2], "LINE3");
    }

    serial.disconnect();
}
