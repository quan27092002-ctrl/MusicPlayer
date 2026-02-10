/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/interfaces/IBoardCommunicator.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for S32K board communication.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IBOARDCOMMUNICATOR_H
#define IBOARDCOMMUNICATOR_H

#include <string>
#include <vector>
#include <cstdint>

namespace Controller {

/**
 * @brief Interface for S32K board communication.
 * 
 * This interface provides access to board connection and
 * communication operations.
 */
class IBoardCommunicator {
public:
    virtual ~IBoardCommunicator() = default;

    /**
     * @brief Connect to S32K board via serial port.
     * @param portName Serial port name (e.g., "/dev/ttyUSB0")
     * @param baudRate Baud rate (default 115200)
     * @return true if connection successful
     */
    virtual bool connectToBoard(const std::string& portName, uint32_t baudRate = 115200) = 0;

    /**
     * @brief Disconnect from the board.
     */
    virtual void disconnectFromBoard() = 0;

    /**
     * @brief Check if connected to board.
     * @return true if connected
     */
    virtual bool isConnectedToBoard() const = 0;
    
    /**
     * @brief Get a list of available serial ports.
     * @return Vector of port names.
     */
    virtual std::vector<std::string> getAvailablePorts() const = 0;
};

} // namespace Controller

#endif // IBOARDCOMMUNICATOR_H
