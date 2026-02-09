/**
 * @file testSerialIOCoverage.cpp
 * @brief Detailed Unit Tests for SerialIOImpl
 * @details Bypass SerialManager to test synchronous read/readLine and thread error handling.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "controller/serialmanager/SerialIOImpl.h"
#include "controller/serialmanager/SerialConnectionImpl.h"
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <chrono>

using namespace Controller;

// ============================================================================
// Helper Infrastructure (PTY)
// ============================================================================
class SerialIOUnitPTYTest : public ::testing::Test {
protected:
    int masterFd = -1;
    std::string slaveName;
    bool ptyAvailable = false;

    void SetUp() override {
        masterFd = posix_openpt(O_RDWR | O_NOCTTY);
        if (masterFd < 0) return;
        
        if (grantpt(masterFd) == 0 && unlockpt(masterFd) == 0) {
            char* name = ptsname(masterFd);
            if (name) {
                slaveName = std::string(name);
                ptyAvailable = true;
            }
        }
    }
    
    void TearDown() override {
        if (masterFd >= 0) close(masterFd);
    }
};

// ============================================================================
// Tests
// ============================================================================

TEST_F(SerialIOUnitPTYTest, ReadSynchronous) {
    if (!ptyAvailable) GTEST_SKIP();
    
    SerialConnectionImpl conn;
    SerialIOImpl io(&conn);
    
    ASSERT_TRUE(conn.connect(slaveName, 115200));
    
    // Write data to master
    write(masterFd, "DATA", 4);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Read directly (lines 47-50 covered)
    uint8_t buffer[10];
    int n = io.read(buffer, sizeof(buffer));
    
    EXPECT_EQ(n, 4);
    buffer[4] = 0;
    EXPECT_STREQ((char*)buffer, "DATA");
    
    conn.disconnect();
}

TEST_F(SerialIOUnitPTYTest, ReadLineSynchronous) {
    if (!ptyAvailable) GTEST_SKIP();
    
    SerialConnectionImpl conn;
    SerialIOImpl io(&conn);
    
    ASSERT_TRUE(conn.connect(slaveName, 115200));
    
    // Write data with newline
    write(masterFd, "Hello\n", 6);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // io.readLine calls internal read byte-by-byte (lines 66-77 covered)
    // It should hit n==1 branch (line 68) multiple times
    // And newline branch (line 69)
    std::string line = io.readLine(1000);
    
    EXPECT_EQ(line, "Hello");
    
    conn.disconnect();
}

// Mock for tricky thread branches
class MockSerialConnectionForIO : public SerialConnectionImpl {
public:
    MOCK_CONST_METHOD0(isConnected, bool());
    // getFileDescriptor is not virtual, so we can't mock it directly specifically for internal calls if not virtual.
    // However, isConnected IS virtual.
};

TEST_F(SerialIOUnitPTYTest, ThreadDisconnectsMidLoop) {
    if (!ptyAvailable) GTEST_SKIP();
    
    MockSerialConnectionForIO mockConn;
    SerialIOImpl io(&mockConn);
    
    ASSERT_TRUE(mockConn.connect(slaveName, 115200));
    
    // Allow extra calls (e.g. from destructor)
    using ::testing::_;
    using ::testing::Return;
    EXPECT_CALL(mockConn, isConnected())
        .WillOnce(Return(true))
        .WillOnce(Return(true))
        .WillRepeatedly(Return(false)); 
        
    io.startReadThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Let thread run
    io.stopReadThread();
}

