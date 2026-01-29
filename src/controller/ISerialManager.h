/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/ISerialManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for serial port communication with S32K board.
 */

#ifndef ISERIALMANAGER_H
#define ISERIALMANAGER_H

#include <string>
#include <functional>
#include <cstdint>

namespace Controller {

/**
 * @brief Serial connection state.
 */
enum class SerialState {
    DISCONNECTED = 0,   ///< Not connected
    CONNECTING = 1,     ///< Connection in progress
    CONNECTED = 2,      ///< Connected and ready
    ERROR = 3           ///< Error state
};

/**
 * @brief Callback type for received data.
 * @param data Received data as string
 */
using SerialDataCallback = std::function<void(const std::string& data)>;

/**
 * @brief Callback type for connection state changes.
 * @param state New connection state
 */
using SerialStateCallback = std::function<void(SerialState state)>;

/**
 * @brief Abstract interface for serial port communication.
 * 
 * Defines the contract for serial communication implementations.
 * Implementations may use platform-specific libraries (e.g., libserial, boost::asio).
 */
class ISerialManager {
public:
    virtual ~ISerialManager() = default;

    // ========================================================================
    // Connection Management
    // ========================================================================

    /**
     * @brief Connect to a serial port.
     * @param portName Port name (e.g., "/dev/ttyUSB0", "COM3")
     * @param baudRate Baud rate (e.g., 9600, 115200)
     * @return true if connection successful
     */
    virtual bool connect(const std::string& portName, uint32_t baudRate) = 0;

    /**
     * @brief Disconnect from the serial port.
     */
    virtual void disconnect() = 0;

    /**
     * @brief Check if currently connected.
     * @return true if connected
     */
    virtual bool isConnected() const = 0;

    /**
     * @brief Get current connection state.
     * @return Current SerialState
     */
    virtual SerialState getState() const = 0;

    // ========================================================================
    // Data Transmission
    // ========================================================================

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

    // ========================================================================
    // Data Reception
    // ========================================================================

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

    // ========================================================================
    // Callbacks
    // ========================================================================

    /**
     * @brief Set callback for received data.
     * @param callback Function to call when data is received
     */
    virtual void setDataCallback(SerialDataCallback callback) = 0;

    /**
     * @brief Set callback for state changes.
     * @param callback Function to call when connection state changes
     */
    virtual void setStateCallback(SerialStateCallback callback) = 0;

    // ========================================================================
    // Port Configuration
    // ========================================================================

    /**
     * @brief Get the current port name.
     * @return Port name, empty if not connected
     */
    virtual std::string getPortName() const = 0;

    /**
     * @brief Get the current baud rate.
     * @return Baud rate, 0 if not connected
     */
    virtual uint32_t getBaudRate() const = 0;

    /**
     * @brief Flush input and output buffers.
     */
    virtual void flush() = 0;
};

} // namespace Controller

#endif // ISERIALMANAGER_H
