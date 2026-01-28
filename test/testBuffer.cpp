/**
 * @file testBuffer.cpp
 * @brief Unit Tests for Buffer class using Google Test framework.
 * @details Covers basic operations, circular buffer behavior, concurrency, and edge cases.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <cstring>
#include "../src/utils/Buffer.h"

using namespace Utils;

/**
 * @brief Test Case: Initial State
 * Scenario: Verify buffer is in correct initial state after construction.
 */
TEST(BufferTest, InitialState) {
    Buffer buffer(1024);

    EXPECT_EQ(buffer.capacity(), 1024);
    EXPECT_EQ(buffer.available(), 0);
}

/**
 * @brief Test Case: Basic Write and Read
 * Scenario: Write data to buffer and read it back.
 */
TEST(BufferTest, BasicWriteRead) {
    Buffer buffer(1024);
    
    uint8_t writeData[] = {1, 2, 3, 4, 5};
    uint8_t readData[5] = {0};

    // Write data
    size_t written = buffer.write(writeData, 5);
    EXPECT_EQ(written, 5);
    EXPECT_EQ(buffer.available(), 5);

    // Read data
    size_t bytesRead = buffer.read(readData, 5);
    EXPECT_EQ(bytesRead, 5);
    EXPECT_EQ(buffer.available(), 0);

    // Verify data integrity
    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(readData[i], writeData[i]);
    }
}

/**
 * @brief Test Case: Partial Read
 * Scenario: Write more data than we read, verify remaining data is correct.
 */
TEST(BufferTest, PartialRead) {
    Buffer buffer(1024);
    
    uint8_t writeData[] = {10, 20, 30, 40, 50};
    uint8_t readData[3] = {0};

    buffer.write(writeData, 5);
    
    // Read only 3 bytes
    size_t bytesRead = buffer.read(readData, 3);
    EXPECT_EQ(bytesRead, 3);
    EXPECT_EQ(buffer.available(), 2); // 2 bytes remaining

    // Verify partial data
    EXPECT_EQ(readData[0], 10);
    EXPECT_EQ(readData[1], 20);
    EXPECT_EQ(readData[2], 30);

    // Read remaining data
    uint8_t remainingData[2] = {0};
    bytesRead = buffer.read(remainingData, 2);
    EXPECT_EQ(bytesRead, 2);
    EXPECT_EQ(remainingData[0], 40);
    EXPECT_EQ(remainingData[1], 50);
}

/**
 * @brief Test Case: Buffer Full Behavior
 * Scenario: Attempt to write more data than buffer capacity.
 */
TEST(BufferTest, BufferFullBehavior) {
    Buffer buffer(10); // Small buffer
    
    uint8_t data[15] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    // Write more than capacity
    size_t written = buffer.write(data, 15);
    EXPECT_EQ(written, 10); // Only 10 bytes should be written
    EXPECT_EQ(buffer.available(), 10);
}

/**
 * @brief Test Case: Read from Empty Buffer
 * Scenario: Attempt to read from an empty buffer.
 */
TEST(BufferTest, ReadFromEmptyBuffer) {
    Buffer buffer(1024);
    
    uint8_t readData[10] = {0};
    size_t bytesRead = buffer.read(readData, 10);
    
    EXPECT_EQ(bytesRead, 0);
    EXPECT_EQ(buffer.available(), 0);
}

/**
 * @brief Test Case: Circular Buffer Wrap-around
 * Scenario: Verify buffer correctly wraps around when write/read positions exceed capacity.
 */
TEST(BufferTest, CircularWrapAround) {
    Buffer buffer(10);
    
    uint8_t writeData1[] = {1, 2, 3, 4, 5, 6, 7, 8};
    uint8_t writeData2[] = {9, 10, 11, 12};
    uint8_t readData[8] = {0};

    // Fill most of the buffer
    buffer.write(writeData1, 8);
    EXPECT_EQ(buffer.available(), 8);

    // Read some data to free up space
    buffer.read(readData, 6);
    EXPECT_EQ(buffer.available(), 2);

    // Write more data (should wrap around)
    size_t written = buffer.write(writeData2, 4);
    EXPECT_EQ(written, 4);
    EXPECT_EQ(buffer.available(), 6);

    // Read all remaining data
    uint8_t allData[6] = {0};
    size_t bytesRead = buffer.read(allData, 6);
    EXPECT_EQ(bytesRead, 6);

    // Verify data: should be {7, 8, 9, 10, 11, 12}
    EXPECT_EQ(allData[0], 7);
    EXPECT_EQ(allData[1], 8);
    EXPECT_EQ(allData[2], 9);
    EXPECT_EQ(allData[3], 10);
    EXPECT_EQ(allData[4], 11);
    EXPECT_EQ(allData[5], 12);
}

/**
 * @brief Test Case: Clear Buffer
 * Scenario: Verify clear() resets the buffer state.
 */
TEST(BufferTest, ClearBuffer) {
    Buffer buffer(1024);
    
    uint8_t data[] = {1, 2, 3, 4, 5};
    buffer.write(data, 5);
    EXPECT_EQ(buffer.available(), 5);

    buffer.clear();

    EXPECT_EQ(buffer.available(), 0);
    
    // Buffer should be usable after clear
    size_t written = buffer.write(data, 5);
    EXPECT_EQ(written, 5);
    EXPECT_EQ(buffer.available(), 5);
}

/**
 * @brief Test Case: Concurrency (Producer-Consumer Pattern)
 * Scenario: One thread writes data, another reads. Verify thread safety.
 */
TEST(BufferTest, ConcurrencyProducerConsumer) {
    Buffer buffer(4096);
    const size_t TOTAL_BYTES = 10000;
    std::atomic<bool> producerFinished{false};
    std::vector<uint8_t> receivedData;
    receivedData.reserve(TOTAL_BYTES);

    // --- THREAD 1: PRODUCER ---
    std::thread producer([&]() {
        size_t bytesSent = 0;
        while (bytesSent < TOTAL_BYTES) {
            uint8_t chunk[100];
            for (int i = 0; i < 100; ++i) {
                chunk[i] = static_cast<uint8_t>((bytesSent + i) % 256);
            }
            
            size_t toWrite = std::min(size_t(100), TOTAL_BYTES - bytesSent);
            size_t written = buffer.write(chunk, toWrite);
            bytesSent += written;
            
            if (written == 0) {
                std::this_thread::yield();
            }
        }
        producerFinished = true;
    });

    // --- THREAD 2: CONSUMER ---
    std::thread consumer([&]() {
        uint8_t readBuf[100];
        while (true) {
            size_t bytesRead = buffer.read(readBuf, 100);
            for (size_t i = 0; i < bytesRead; ++i) {
                receivedData.push_back(readBuf[i]);
            }
            
            if (producerFinished && buffer.available() == 0) {
                break;
            }
            
            if (bytesRead == 0) {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    // --- VERIFICATION ---
    EXPECT_EQ(receivedData.size(), TOTAL_BYTES);

    // Verify data integrity
    for (size_t i = 0; i < TOTAL_BYTES; ++i) {
        EXPECT_EQ(receivedData[i], static_cast<uint8_t>(i % 256));
    }
}

/**
 * @brief Test Case: Multiple Write Operations
 * Scenario: Perform multiple consecutive writes and one read.
 */
TEST(BufferTest, MultipleWrites) {
    Buffer buffer(1024);
    
    uint8_t data1[] = {1, 2, 3};
    uint8_t data2[] = {4, 5, 6};
    uint8_t data3[] = {7, 8, 9, 10};

    buffer.write(data1, 3);
    buffer.write(data2, 3);
    buffer.write(data3, 4);

    EXPECT_EQ(buffer.available(), 10);

    uint8_t readData[10] = {0};
    size_t bytesRead = buffer.read(readData, 10);
    
    EXPECT_EQ(bytesRead, 10);
    for (int i = 0; i < 10; ++i) {
        EXPECT_EQ(readData[i], i + 1);
    }
}

/**
 * @brief Test Case: Large Data Transfer
 * Scenario: Write and read large chunks of data.
 */
TEST(BufferTest, LargeDataTransfer) {
    const size_t BUFFER_SIZE = 64 * 1024; // 64KB
    Buffer buffer(BUFFER_SIZE);
    
    std::vector<uint8_t> largeData(BUFFER_SIZE);
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
        largeData[i] = static_cast<uint8_t>(i % 256);
    }

    size_t written = buffer.write(largeData.data(), BUFFER_SIZE);
    EXPECT_EQ(written, BUFFER_SIZE);
    EXPECT_EQ(buffer.available(), BUFFER_SIZE);

    std::vector<uint8_t> readData(BUFFER_SIZE);
    size_t bytesRead = buffer.read(readData.data(), BUFFER_SIZE);
    
    EXPECT_EQ(bytesRead, BUFFER_SIZE);
    EXPECT_EQ(buffer.available(), 0);

    // Verify data integrity
    EXPECT_EQ(largeData, readData);
}
