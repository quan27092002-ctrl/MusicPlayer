/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/serialmanager/interfaces/ISerialIO.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for serial I/O operations.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef ISERIALIO_H
#define ISERIALIO_H

#include <string>
#include <cstdint>
#include <cstddef>
#include <functional>

namespace Controller {

/**
 * @brief Callback type for received data.
 */
using SerialDataCallback = std::function<void(const std::string& data)>;

/**
 * @brief Interface for serial I/O operations.
 * 
 * Provides access to send and receive data operations.
 */
class ISerialIO {
public:
    virtual ~ISerialIO() = default;

    /**
     * @brief Send data through serial port.
     * @param data Data to send
     * @return Number of bytes sent, -1 on error
     */
    virtual int send(const std::string& data) = 0;

    /**
     * @brief Send raw bytes through serial port.
     * @param data Pointer to data buffer
     * @param length Number of bytes to send
     * @return Number of bytes sent, -1 on error
     */
    virtual int sendBytes(const uint8_t* data, size_t length) = 0;

    /**
     * @brief Read available data from serial port (non-blocking).
     * @param buffer Buffer to store received data
     * @param maxLength Maximum bytes to read
     * @return Number of bytes read, -1 on error
     */
    virtual int read(uint8_t* buffer, size_t maxLength) = 0;

    /**
     * @brief Read a line of text (blocking until newline or timeout).
     * @param timeout Timeout in milliseconds (0 = no timeout)
     * @return Received line, empty string on timeout/error
     */
    virtual std::string readLine(uint32_t timeout = 0) = 0;

    /**
     * @brief Check if data is available to read.
     * @return Number of bytes available
     */
    virtual size_t available() const = 0;

    /**
     * @brief Flush input and output buffers.
     */
    virtual void flush() = 0;

    /**
     * @brief Set callback for received data.
     * @param callback Function to call when data is received
     */
    virtual void setDataCallback(SerialDataCallback callback) = 0;
};

} // namespace Controller

#endif // ISERIALIO_H
