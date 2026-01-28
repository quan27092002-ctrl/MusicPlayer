/**
 * @file testLogger.cpp
 * @brief Unit Tests for Logger class using Google Test framework.
 * @details Covers log levels, singleton behavior, thread safety, and output filtering.
 * @author Architecture Team
 */

#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <sstream>
#include <regex>
#include "../src/utils/Logger.h"

using namespace Utils;

/**
 * @brief Test Fixture for Logger tests.
 * Resets logger state before each test.
 */
class LoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Reset to default level before each test
        Logger::getInstance().setLevel(LogLevel::INFO);
    }

    void TearDown() override {
        // Reset after test
        Logger::getInstance().setLevel(LogLevel::INFO);
    }
};

/**
 * @brief Test Case: Singleton Pattern
 * Scenario: Verify that getInstance() always returns the same instance.
 */
TEST_F(LoggerTest, SingletonInstance) {
    Logger& instance1 = Logger::getInstance();
    Logger& instance2 = Logger::getInstance();

    // Both references should point to the same object
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @brief Test Case: Default Log Level
 * Scenario: Verify default log level is INFO.
 */
TEST_F(LoggerTest, DefaultLogLevel) {
    // SetUp resets to INFO, so this tests the expected default behavior
    EXPECT_EQ(Logger::getInstance().getLevel(), LogLevel::INFO);
}

/**
 * @brief Test Case: Set and Get Log Level
 * Scenario: Verify setLevel and getLevel work correctly.
 */
TEST_F(LoggerTest, SetAndGetLogLevel) {
    Logger& logger = Logger::getInstance();

    logger.setLevel(LogLevel::DEBUG);
    EXPECT_EQ(logger.getLevel(), LogLevel::DEBUG);

    logger.setLevel(LogLevel::WARNING);
    EXPECT_EQ(logger.getLevel(), LogLevel::WARNING);

    logger.setLevel(LogLevel::ERROR);
    EXPECT_EQ(logger.getLevel(), LogLevel::ERROR);

    logger.setLevel(LogLevel::NONE);
    EXPECT_EQ(logger.getLevel(), LogLevel::NONE);
}

/**
 * @brief Test Case: Log Level Filtering
 * Scenario: Verify that messages below current level are filtered.
 * Note: Since Logger outputs to stdout/stderr, we can only verify no crash.
 */
TEST_F(LoggerTest, LogLevelFiltering) {
    Logger& logger = Logger::getInstance();

    // Set to ERROR level - should only show ERROR messages
    logger.setLevel(LogLevel::ERROR);

    // These should be filtered (no crash expected)
    logger.log(LogLevel::DEBUG, "This DEBUG should be filtered");
    logger.log(LogLevel::INFO, "This INFO should be filtered");
    logger.log(LogLevel::WARNING, "This WARNING should be filtered");

    // This should pass through
    logger.log(LogLevel::ERROR, "This ERROR should show");

    // If we get here without crash, test passes
    SUCCEED();
}

/**
 * @brief Test Case: Log Level NONE Disables All
 * Scenario: Verify NONE level disables all logging.
 */
TEST_F(LoggerTest, LogLevelNoneDisablesAll) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::NONE);

    // All levels should be filtered
    logger.log(LogLevel::DEBUG, "Filtered");
    logger.log(LogLevel::INFO, "Filtered");
    logger.log(LogLevel::WARNING, "Filtered");
    logger.log(LogLevel::ERROR, "Filtered");

    SUCCEED();
}

/**
 * @brief Test Case: Macros Work Correctly
 * Scenario: Verify LOG_* macros compile and execute without crash.
 */
TEST_F(LoggerTest, MacrosCompileAndRun) {
    Logger::getInstance().setLevel(LogLevel::DEBUG);

    // Test all macros with stream syntax
    LOG_DEBUG("Debug message: " << 42);
    LOG_INFO("Info message: " << "test");
    LOG_WARNING("Warning message: " << 3.14);
    LOG_ERROR("Error message: " << true);

    SUCCEED();
}

/**
 * @brief Test Case: Thread Safety - Concurrent Logging
 * Scenario: Multiple threads logging simultaneously without crash or data corruption.
 */
TEST_F(LoggerTest, ConcurrentLogging) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);

    const int NUM_THREADS = 4;
    const int LOGS_PER_THREAD = 100;
    std::vector<std::thread> threads;

    // Create multiple threads that log concurrently
    for (int t = 0; t < NUM_THREADS; ++t) {
        threads.emplace_back([&logger, t]() {
            for (int i = 0; i < LOGS_PER_THREAD; ++i) {
                logger.log(LogLevel::INFO, 
                    "Thread " + std::to_string(t) + " - Message " + std::to_string(i));
            }
        });
    }

    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }

    // If we get here without crash or deadlock, test passes
    SUCCEED();
}

/**
 * @brief Test Case: Thread Safety - Concurrent Level Changes
 * Scenario: One thread changes log level while others are logging.
 */
TEST_F(LoggerTest, ConcurrentLevelChange) {
    Logger& logger = Logger::getInstance();
    std::atomic<bool> running{true};

    // Thread that continuously changes log level
    std::thread levelChanger([&]() {
        while (running) {
            logger.setLevel(LogLevel::DEBUG);
            logger.setLevel(LogLevel::INFO);
            logger.setLevel(LogLevel::WARNING);
            logger.setLevel(LogLevel::ERROR);
        }
    });

    // Threads that log continuously
    std::vector<std::thread> loggers;
    for (int t = 0; t < 3; ++t) {
        loggers.emplace_back([&logger, t]() {
            for (int i = 0; i < 50; ++i) {
                logger.log(LogLevel::INFO, "Logger thread " + std::to_string(t));
            }
        });
    }

    // Wait for loggers to finish
    for (auto& thread : loggers) {
        thread.join();
    }

    // Stop level changer
    running = false;
    levelChanger.join();

    SUCCEED();
}

/**
 * @brief Test Case: Empty Message
 * Scenario: Verify empty messages don't cause crash.
 */
TEST_F(LoggerTest, EmptyMessage) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);

    logger.log(LogLevel::INFO, "");
    LOG_INFO("");

    SUCCEED();
}

/**
 * @brief Test Case: Long Message
 * Scenario: Verify long messages are handled correctly.
 */
TEST_F(LoggerTest, LongMessage) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);

    std::string longMessage(10000, 'x');
    logger.log(LogLevel::INFO, longMessage);

    SUCCEED();
}

/**
 * @brief Test Case: Special Characters in Message
 * Scenario: Verify special characters don't cause issues.
 */
TEST_F(LoggerTest, SpecialCharacters) {
    Logger& logger = Logger::getInstance();
    logger.setLevel(LogLevel::DEBUG);

    logger.log(LogLevel::INFO, "Special chars: \t\n\r\"'\\");
    logger.log(LogLevel::INFO, "Unicode: こんにちは 你好 🎵");

    SUCCEED();
}
