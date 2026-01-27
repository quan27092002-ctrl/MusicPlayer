/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/IThreadSafeQueue.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Abstract Interface for Thread-Safe Communication.
 */

#ifndef I_THREAD_SAFE_QUEUE_H
#define I_THREAD_SAFE_QUEUE_H

namespace Utils {

template <typename T>
class IThreadSafeQueue {
public:
    virtual ~IThreadSafeQueue() = default;

    /**
     * @brief Pushes a new item into the queue.
     * Implementation must be thread-safe.
     */
    virtual void push(const T& value) = 0;

    /**
     * @brief Non-blocking attempt to pop an item.
     * @param value Reference to store the popped item if successful.
     * @return true if an item was retrieved, false if queue was empty.
     */
    virtual bool tryPop(T& value) = 0; 

    /**
     * @brief Blocking wait until an item is available.
     * @param value Reference to store the popped item.
     */
    virtual void waitAndPop(T& value) = 0;

    /**
     * @brief Check if queue is empty.
     */
    virtual bool empty() const = 0;
};

} // namespace Utils

#endif // I_THREAD_SAFE_QUEUE_H