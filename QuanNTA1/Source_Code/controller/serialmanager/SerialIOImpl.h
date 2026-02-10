/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/serialmanager/SerialIOImpl.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of ISerialIO.
 */

#ifndef SERIALIO_IMPL_H
#define SERIALIO_IMPL_H

#include "interfaces/ISerialIO.h"
#include "SerialConnectionImpl.h"
#include <thread>
#include <atomic>
#include <mutex>

namespace Controller {

/**
 * @brief Concrete implementation of ISerialIO.
 *
 * Manages serial data transmission and reception.
 */
class SerialIOImpl : public ISerialIO {
public:
    SerialIOImpl(SerialConnectionImpl* connection);
    ~SerialIOImpl() override;

    // ISerialIO interface
    int send(const std::string& data) override;
    int sendBytes(const uint8_t* data, size_t length) override;
    int read(uint8_t* buffer, size_t maxLength) override;
    std::string readLine(uint32_t timeoutMs = 1000) override;
    size_t available() const override;
    void flush() override;
    void setDataCallback(SerialDataCallback callback) override;
    
    // Thread control
    void startReadThread();
    void stopReadThread();

private:
    SerialConnectionImpl* mConnection;
    SerialDataCallback mDataCallback;
    mutable std::mutex mCallbackMutex;
    std::thread mReadThread;
    std::atomic<bool> mRunning;
    
    void readThreadFunc();
    void notifyDataReceived(const std::string& data);
};

} // namespace Controller

#endif // SERIALIO_IMPL_H
