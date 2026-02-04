/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/serialmanager/SerialConnectionImpl.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of ISerialConnection.
 */

#ifndef SERIALCONNECTION_IMPL_H
#define SERIALCONNECTION_IMPL_H

#include "interfaces/ISerialConnection.h"
#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <functional>

namespace Controller {

/**
 * @brief Concrete implementation of ISerialConnection.
 *
 * Manages serial port connection lifecycle.
 */
class SerialConnectionImpl : public ISerialConnection {
public:
    SerialConnectionImpl();
    ~SerialConnectionImpl() override;

    // ISerialConnection interface
    bool connect(const std::string& portName, uint32_t baudRate) override;
    void disconnect() override;
    bool isConnected() const override;
    SerialState getState() const override;
    void setStateCallback(SerialStateCallback callback) override;
    std::string getPortName() const override;
    uint32_t getBaudRate() const override;
    
    // Additional methods
    int getFileDescriptor() const;
    void setState(SerialState state);
    void notifyStateChange(SerialState state);

private:
    int mFileDescriptor;
    std::string mPortName;
    uint32_t mBaudRate;
    std::atomic<SerialState> mState;
    SerialStateCallback mStateCallback;
    mutable std::mutex mMutex;
    
    bool configureBaudRate(int fd, uint32_t baudRate);
};

} // namespace Controller

#endif // SERIALCONNECTION_IMPL_H
