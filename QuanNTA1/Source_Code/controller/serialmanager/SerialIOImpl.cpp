/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/serialmanager/SerialIOImpl.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of SerialIOImpl.
 */

#include "SerialIOImpl.h"
#include <unistd.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <chrono>
#include <cerrno>

namespace Controller {

SerialIOImpl::SerialIOImpl(SerialConnectionImpl* connection)
    : mConnection(connection)
    , mDataCallback(nullptr)
    , mRunning(false)
{}

SerialIOImpl::~SerialIOImpl() {
    stopReadThread();
}

int SerialIOImpl::send(const std::string& data) {
    return sendBytes(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
}

int SerialIOImpl::sendBytes(const uint8_t* data, size_t length) {
    if (!mConnection || !mConnection->isConnected()) {
        return -1;
    }
    
    int fd = mConnection->getFileDescriptor();
    if (fd < 0) return -1;
    
    return write(fd, data, length);
}

int SerialIOImpl::read(uint8_t* buffer, size_t maxLength) {
    if (!mConnection || !mConnection->isConnected()) {
        return -1;
    }
    
    int fd = mConnection->getFileDescriptor();
    if (fd < 0) return -1;
    
    return ::read(fd, buffer, maxLength);
}

std::string SerialIOImpl::readLine(uint32_t timeoutMs) {
    if (!mConnection || !mConnection->isConnected()) {
        return "";
    }

    int fd = mConnection->getFileDescriptor();
    if (fd < 0) return "";

    std::string line;
    char ch;
    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        int n = ::read(fd, &ch, 1);
        
        if (n == 1) {
            if (ch == '\n') {
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                return line;
            }
            line += ch;
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            return "";
        }

        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime
            ).count();
            if (elapsed >= timeoutMs) {
                return "";
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

size_t SerialIOImpl::available() const {
    if (!mConnection || !mConnection->isConnected()) {
        return 0;
    }
    
    int fd = mConnection->getFileDescriptor();
    if (fd < 0) return 0;

    int bytes = 0;
    ioctl(fd, FIONREAD, &bytes);
    return static_cast<size_t>(bytes);
}

void SerialIOImpl::flush() {
    if (!mConnection || !mConnection->isConnected()) {
        return;
    }
    
    int fd = mConnection->getFileDescriptor();
    if (fd >= 0) {
        tcflush(fd, TCIOFLUSH);
    }
}

void SerialIOImpl::setDataCallback(SerialDataCallback callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mDataCallback = callback;
}

void SerialIOImpl::startReadThread() {
    if (mRunning.load()) return;
    
    mRunning.store(true);
    mReadThread = std::thread(&SerialIOImpl::readThreadFunc, this);
}

void SerialIOImpl::stopReadThread() {
    mRunning.store(false);
    if (mReadThread.joinable()) {
        mReadThread.join();
    }
}

void SerialIOImpl::readThreadFunc() {
    char buffer[256];
    std::string lineBuffer;

    while (mRunning.load()) {
        if (!mConnection || !mConnection->isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int fd = mConnection->getFileDescriptor();
        if (fd < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        int n = ::read(fd, buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            buffer[n] = '\0';
            lineBuffer += std::string(buffer, n);

            size_t pos;
            while ((pos = lineBuffer.find('\n')) != std::string::npos) {
                std::string line = lineBuffer.substr(0, pos);
                lineBuffer.erase(0, pos + 1);
                
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                
                if (!line.empty()) {
                    notifyDataReceived(line);
                }
            }
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            mConnection->setState(SerialState::ERROR);
            mConnection->notifyStateChange(SerialState::ERROR);
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void SerialIOImpl::notifyDataReceived(const std::string& data) {
    SerialDataCallback cb;
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        cb = mDataCallback;
    }
    if (cb) {
        cb(data);
    }
}

} // namespace Controller
