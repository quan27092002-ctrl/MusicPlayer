/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/SerialManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: POSIX-based implementation of ISerialManager using termios.
 */

#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include "ISerialManager.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>

namespace Controller {

/**
 * @brief POSIX termios-based serial port implementation.
 * 
 * Uses Linux termios API for serial communication.
 * Runs a background thread for receiving data.
 */
class SerialManager : public ISerialManager {
private:
    int mFileDescriptor;                    ///< Serial port file descriptor
    std::string mPortName;                  ///< Current port name
    uint32_t mBaudRate;                     ///< Current baud rate
    
    std::atomic<SerialState> mState;        ///< Connection state
    
    SerialDataCallback mDataCallback;       ///< Data received callback
    SerialStateCallback mStateCallback;     ///< State change callback
    mutable std::mutex mCallbackMutex;      ///< Protects callbacks
    
    std::thread mReadThread;                ///< Background read thread
    std::atomic<bool> mRunning;             ///< Thread running flag
    
    // Helper methods
    void readThreadFunc();
    void notifyStateChange(SerialState newState);
    void notifyDataReceived(const std::string& data);
    bool configureBaudRate(int fd, uint32_t baudRate);

public:
    /**
     * @brief Constructor.
     */
    SerialManager();

    /**
     * @brief Destructor - ensures cleanup.
     */
    ~SerialManager() override;

    // Delete copy (thread resources are unique)
    SerialManager(const SerialManager&) = delete;
    SerialManager& operator=(const SerialManager&) = delete;

    // ========================================================================
    // ISerialManager Interface Implementation
    // ========================================================================

    // Connection Management
    bool connect(const std::string& portName, uint32_t baudRate) override;
    void disconnect() override;
    bool isConnected() const override;
    SerialState getState() const override;

    // Data Transmission
    int send(const std::string& data) override;
    int sendBytes(const uint8_t* data, size_t length) override;

    // Data Reception
    int read(uint8_t* buffer, size_t maxLength) override;
    std::string readLine(uint32_t timeout = 0) override;
    size_t available() const override;

    // Callbacks
    void setDataCallback(SerialDataCallback callback) override;
    void setStateCallback(SerialStateCallback callback) override;

    // Port Configuration
    std::string getPortName() const override;
    uint32_t getBaudRate() const override;
    void flush() override;
};

} // namespace Controller

#endif // SERIALMANAGER_H
