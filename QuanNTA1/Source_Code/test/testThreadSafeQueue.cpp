/**
 * @file test_ThreadSafeQueue.cpp
 * @brief Unit Tests for ThreadSafeQueue class using Google Test framework.
 * @details Covers basic operations, concurrency (producer-consumer), and blocking behavior.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <atomic>
#include <string>
#include <chrono>
#include "../src/utils/ThreadSafeQueue.h"

using namespace Utils;

/**
 * @brief Test Case: Basic Operations
 * Scenarios: Check empty state, push items, tryPop (non-blocking).
 */
TEST(ThreadSafeQueueTest, BasicOperations) {
    ThreadSafeQueue<int> queue;

    // 1. Initial state should be empty
    EXPECT_TRUE(queue.empty());

    // 2. Push items
    queue.push(10);
    queue.push(20);
    EXPECT_FALSE(queue.empty());

    // 3. Test tryPop (Non-blocking retrieval)
    int value = 0;
    bool success = queue.tryPop(value);
    
    EXPECT_TRUE(success);
    EXPECT_EQ(value, 10); // FIFO verification (First In, First Out)

    queue.tryPop(value);
    EXPECT_EQ(value, 20);

    // 4. Test popping from empty queue
    success = queue.tryPop(value);
    EXPECT_FALSE(success); // Should fail safely
    EXPECT_TRUE(queue.empty());
}

/**
 * @brief Test Case: Concurrency (Producer-Consumer Pattern)
 * Scenarios: One thread pushes 1000 items, another reads them. 
 * goal: Ensure no data loss and correct ordering under race conditions.
 */
TEST(ThreadSafeQueueTest, ConcurrencyPushPop) {
    ThreadSafeQueue<int> queue;
    const int ITEM_COUNT = 1000;
    std::atomic<bool> producerFinished{false};
    std::vector<int> receivedData;

    // --- THREAD 1: PRODUCER ---
    std::thread producer([&]() {
        for (int i = 0; i < ITEM_COUNT; ++i) {
            queue.push(i);
            // Yield CPU to increase chance of context switching (Simulate race condition)
            if (i % 10 == 0) std::this_thread::yield(); 
        }
        producerFinished = true;
    });

    // --- THREAD 2: CONSUMER ---
    std::thread consumer([&]() {
        int val;
        while (true) {
            // Block and wait for data
            queue.waitAndPop(val);
            receivedData.push_back(val);

            // Exit condition: Producer is done AND Queue is drained
            // Note: Checking only 'producerFinished' is unsafe as queue might still have data
            if (producerFinished && queue.empty()) {
                break;
            }
        }
    });

    // Wait for threads to complete
    producer.join();
    consumer.join();

    // --- VERIFICATION ---
    // 1. Validate total count
    EXPECT_EQ(receivedData.size(), ITEM_COUNT);

    // 2. Validate data integrity and order
    for (int i = 0; i < ITEM_COUNT; ++i) {
        EXPECT_EQ(receivedData[i], i);
    }
}

/**
 * @brief Test Case: Blocking Behavior
 * Scenario: Consumer attempts to pop from empty queue and must wait (sleep).
 */
TEST(ThreadSafeQueueTest, BlockingBehavior) {
    ThreadSafeQueue<std::string> queue;
    std::string result;
    
    // Create Consumer thread first
    std::thread consumer([&]() {
        // This call must block until data is available
        queue.waitAndPop(result); 
    });

    // Main thread sleeps to ensure Consumer enters 'wait' state
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Producer pushes data
    queue.push("Hello");

    consumer.join();

    EXPECT_EQ(result, "Hello");
}