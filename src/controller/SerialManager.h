/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/SerialManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: POSIX-based implementation of ISerialManager using termios.
 *              Facade class using composition following SOLID principles.
 */

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include "ISerialManager.h"
#include "serialmanager/SerialConnectionImpl.h"
#include "serialmanager/SerialIOImpl.h"
#include <memory>

namespace Controller {

/**
 * @brief POSIX-based serial manager implementation (Facade).
 * 
 * Uses composition to delegate to specialized components:
 * - SerialConnectionImpl: Connection management
 * - SerialIOImpl: I/O operations
 */
class SerialManager : public ISerialManager {
public:
    SerialManager();
    ~SerialManager() override;

    // Delete copy
    SerialManager(const SerialManager&) = delete;
    SerialManager& operator=(const SerialManager&) = delete;

    // ========================================================================
    // ISerialConnection (delegates to mConnection)
    // ========================================================================
    bool connect(const std::string& portName, uint32_t baudRate) override;
    void disconnect() override;
    bool isConnected() const override;
    SerialState getState() const override;
    void setStateCallback(SerialStateCallback callback) override;
    std::string getPortName() const override;
    uint32_t getBaudRate() const override;

    // ========================================================================
    // ISerialIO (delegates to mIO)
    // ========================================================================
    int send(const std::string& data) override;
    int sendBytes(const uint8_t* data, size_t length) override;
    int read(uint8_t* buffer, size_t maxLength) override;
    std::string readLine(uint32_t timeoutMs = 1000) override;
    size_t available() const override;
    void flush() override;
    void setDataCallback(SerialDataCallback callback) override;

private:
    std::unique_ptr<SerialConnectionImpl> mConnection;
    std::unique_ptr<SerialIOImpl> mIO;
};

} // namespace Controller

#endif // SERIALMANAGER_H
