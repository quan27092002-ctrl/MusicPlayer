/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/Buffer.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete declaration of Buffer. 
 * Implements circular buffering logic for uint8_t audio streams.
 */

#ifndef BUFFER_H
#define BUFFER_H

#include "IBuffer.h"
#include <vector>
#include <mutex>
#include <cstdint>

namespace Utils {

class Buffer : public IBuffer {
private:
    size_t getAvailableInternal() const;
    bool isFullInternal() const;
    bool isEmptyInternal() const;

    std::vector<uint8_t> mBuffer; // Storage
    size_t mHead;                 // Write Index (Producer)
    size_t mTail;                 // Read Index (Consumer)
    bool mIsFull;                 // Full flag
    size_t mCapacity;             // Total size
    
    mutable std::mutex mMutex;    // Protects shared access

public:
    /**
     * @brief Constructor
     * @param capacityBytes Size of buffer in bytes (Default: 1MB for Audio)
     */
    explicit Buffer(size_t capacityBytes = 1024 * 1024);
    
    ~Buffer() override = default;

    // Disable Copying (Unique Resource)
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    /**
     * @brief Writes data into the buffer (Thread-safe).
     * @return Bytes actually written.
     */
    size_t write(const uint8_t* data, size_t len) override;

    /**
     * @brief Reads data from the buffer (Thread-safe).
     * @return Bytes actually read.
     */
    size_t read(uint8_t* dest, size_t len) override;

    /**
     * @brief Resets the buffer.
     */
    void clear() override;

    /**
     * @brief Returns currently readable bytes.
     */
    size_t available() const override;

    /**
     * @brief Returns total capacity.
     */
    size_t capacity() const override;

};

} // namespace Utils

#endif // BUFFER_H