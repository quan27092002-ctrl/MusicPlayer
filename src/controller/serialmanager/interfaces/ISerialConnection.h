/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/serialmanager/interfaces/ISerialConnection.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for serial connection management.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef ISERIALCONNECTION_H
#define ISERIALCONNECTION_H

#include <string>
#include <cstdint>
#include <functional>

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
 * @brief Callback type for connection state changes.
 */
using SerialStateCallback = std::function<void(SerialState state)>;

/**
 * @brief Interface for serial connection management.
 * 
 * Provides access to connect, disconnect, and state operations.
 */
class ISerialConnection {
public:
    virtual ~ISerialConnection() = default;

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
     * @brief Set callback for state changes.
     * @param callback Function to call when connection state changes
     */
    virtual void setStateCallback(SerialStateCallback callback) = 0;
};

} // namespace Controller

#endif // ISERIALCONNECTION_H
