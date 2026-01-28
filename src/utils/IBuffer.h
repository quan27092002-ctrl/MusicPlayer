/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/utils/IBuffer.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Abstract Interface for Audio Data Buffering.
 * Defines the contract for writing (Producer) and reading (Consumer) byte streams.
 */

#ifndef I_BUFFER_H
#define I_BUFFER_H

#include <cstdint>
#include <cstddef> 

namespace Utils {

class IBuffer {
public:
    virtual ~IBuffer() = default;

    /**
     * @brief Write data into the buffer.
     * @param data Pointer to the source data array.
     * @param len Number of bytes to write.
     * @return Number of bytes actually written (may be less than len if buffer is full).
     */
    virtual size_t write(const uint8_t* data, size_t len) = 0;

    /**
     * @brief Read data from the buffer.
     * @param dest Pointer to the destination array.
     * @param len Number of bytes requested.
     * @return Number of bytes actually read (may be less than len if buffer is empty).
     */
    virtual size_t read(uint8_t* dest, size_t len) = 0;

    /**
     * @brief Clear the buffer (reset head/tail indices).
     */
    virtual void clear() = 0;

    /**
     * @brief Get the number of bytes currently available for reading.
     */
    virtual size_t available() const = 0;

    /**
     * @brief Get the total capacity of the buffer in bytes.
     */
    virtual size_t capacity() const = 0;
};

} // namespace Utils

#endif // I_BUFFER_H