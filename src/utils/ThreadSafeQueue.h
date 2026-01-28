/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/ThreadSafeQueue.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete Implementation of Thread-Safe Queue using Mutex & CondVar.
 */

#ifndef THREAD_SAFE_QUEUE_H
#define THREAD_SAFE_QUEUE_H

#include "IThreadSafeQueue.h"
#include <queue>
#include <mutex>
#include <condition_variable>

namespace Utils {

template <typename T>
class ThreadSafeQueue : public IThreadSafeQueue<T> {
private:
    std::queue<T> mQueue;             // Standard STL queue
    mutable std::mutex mMutex;        // Mutex for protection
    std::condition_variable mCondVar; // Condition variable for signaling
public:
    ThreadSafeQueue() = default;
    ~ThreadSafeQueue() override = default;

    // Delete Copy Constructor & Assignment Operator to ensure thread safety
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    /**
     * @brief Push data into the queue (Producer).
     * Uses lock_guard for scoped locking.
     */
    void push(const T& value) override {
        std::lock_guard<std::mutex> lock(mMutex);
        mQueue.push(value);
        mCondVar.notify_one(); // Wake up one waiting thread (if any)
    }

    /**
     * @brief Try to retrieve data without blocking (Non-blocking).
     * @return true if data was retrieved, false if queue was empty.
     */
    bool tryPop(T& value) override {
        std::lock_guard<std::mutex> lock(mMutex);
        if (mQueue.empty()) {
            return false;
        }
        
        value = std::move(mQueue.front());
        mQueue.pop();
        return true;
    }

    /**
     * @brief Wait until data is available (Consumer).
     * Blocking call: The thread will sleep here to save CPU.
     */
    void waitAndPop(T& value) override {
        std::unique_lock<std::mutex> lock(mMutex);
        
        // Wait Loop to handle Spurious Wakeup
        // The thread unlocks mutex and sleeps until mQueue is not empty
        mCondVar.wait(lock, [this] { return !mQueue.empty(); });//return 1 -> unlock -> sleep

        value = std::move(mQueue.front());
        mQueue.pop();
    }

    /**
     * @brief Check if queue is empty (Thread-safe).
     */
    bool empty() const override {
        std::lock_guard<std::mutex> lock(mMutex);
        return mQueue.empty();
    }
};

} // namespace Utils

#endif // THREAD_SAFE_QUEUE_H