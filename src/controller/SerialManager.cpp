/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/SerialManager.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of SerialManager using POSIX termios.
 */

#include "SerialManager.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <cstring>
#include <chrono>

namespace Controller {

// ============================================================================
// Constructor / Destructor
// ============================================================================

SerialManager::SerialManager()
    : mFileDescriptor(-1)
    , mPortName("")
    , mBaudRate(0)
    , mState(SerialState::DISCONNECTED)
    , mDataCallback(nullptr)
    , mStateCallback(nullptr)
    , mRunning(false) {
}

SerialManager::~SerialManager() {
    disconnect();
}

// ============================================================================
// Private Helpers
// ============================================================================

void SerialManager::notifyStateChange(SerialState newState) {
    SerialStateCallback cb;
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        cb = mStateCallback;
    }
    if (cb) {
        cb(newState);
    }
}

void SerialManager::notifyDataReceived(const std::string& data) {
    SerialDataCallback cb;
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        cb = mDataCallback;
    }
    if (cb) {
        cb(data);
    }
}

bool SerialManager::configureBaudRate(int fd, uint32_t baudRate) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) {
        return false;
    }

    // Map baud rate to termios constant
    speed_t speed;
    switch (baudRate) {
        case 9600:   speed = B9600; break;
        case 19200:  speed = B19200; break;
        case 38400:  speed = B38400; break;
        case 57600:  speed = B57600; break;
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        default:     return false;  // Unsupported baud rate
    }

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // 8N1 mode (8 data bits, no parity, 1 stop bit)
    tty.c_cflag &= ~PARENB;        // No parity
    tty.c_cflag &= ~CSTOPB;        // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;            // 8 data bits
    tty.c_cflag &= ~CRTSCTS;       // No hardware flow control
    tty.c_cflag |= CREAD | CLOCAL; // Enable receiver, ignore modem control

    // Raw mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;

    // Non-blocking read
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;  // 0.1 second timeout

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        return false;
    }

    return true;
}

void SerialManager::readThreadFunc() {
    char buffer[256];
    std::string lineBuffer;

    while (mRunning.load()) {
        int n = ::read(mFileDescriptor, buffer, sizeof(buffer) - 1);
        
        if (n > 0) {
            buffer[n] = '\0';
            lineBuffer += std::string(buffer, n);

            // Process complete lines
            size_t pos;
            while ((pos = lineBuffer.find('\n')) != std::string::npos) {
                std::string line = lineBuffer.substr(0, pos);
                lineBuffer.erase(0, pos + 1);
                
                // Remove carriage return if present
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                
                if (!line.empty()) {
                    notifyDataReceived(line);
                }
            }
        } else if (n < 0) {
            // Error occurred
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                mState.store(SerialState::ERROR);
                notifyStateChange(SerialState::ERROR);
                break;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// ============================================================================
// Connection Management
// ============================================================================

bool SerialManager::connect(const std::string& portName, uint32_t baudRate) {
    if (isConnected()) {
        disconnect();
    }

    mState.store(SerialState::CONNECTING);
    notifyStateChange(SerialState::CONNECTING);

    // Open serial port
    mFileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mFileDescriptor < 0) {
        mState.store(SerialState::ERROR);
        notifyStateChange(SerialState::ERROR);
        return false;
    }

    // Configure baud rate and settings
    if (!configureBaudRate(mFileDescriptor, baudRate)) {
        close(mFileDescriptor);
        mFileDescriptor = -1;
        mState.store(SerialState::ERROR);
        notifyStateChange(SerialState::ERROR);
        return false;
    }

    mPortName = portName;
    mBaudRate = baudRate;
    mState.store(SerialState::CONNECTED);

    // Start read thread
    mRunning.store(true);
    mReadThread = std::thread(&SerialManager::readThreadFunc, this);

    notifyStateChange(SerialState::CONNECTED);
    return true;
}

void SerialManager::disconnect() {
    if (!isConnected()) {
        return;
    }

    // Stop read thread
    mRunning.store(false);
    if (mReadThread.joinable()) {
        mReadThread.join();
    }

    // Close port
    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
        mFileDescriptor = -1;
    }

    mPortName.clear();
    mBaudRate = 0;
    mState.store(SerialState::DISCONNECTED);
    notifyStateChange(SerialState::DISCONNECTED);
}

bool SerialManager::isConnected() const {
    return mState.load() == SerialState::CONNECTED;
}

SerialState SerialManager::getState() const {
    return mState.load();
}

// ============================================================================
// Data Transmission
// ============================================================================

int SerialManager::send(const std::string& data) {
    return sendBytes(reinterpret_cast<const uint8_t*>(data.c_str()), data.length());
}

int SerialManager::sendBytes(const uint8_t* data, size_t length) {
    if (!isConnected() || mFileDescriptor < 0) {
        return -1;
    }

    int n = write(mFileDescriptor, data, length);
    return n;
}

// ============================================================================
// Data Reception
// ============================================================================

int SerialManager::read(uint8_t* buffer, size_t maxLength) {
    if (!isConnected() || mFileDescriptor < 0) {
        return -1;
    }

    int n = ::read(mFileDescriptor, buffer, maxLength);
    return n;
}

std::string SerialManager::readLine(uint32_t timeout) {
    if (!isConnected()) {
        return "";
    }

    std::string line;
    char ch;
    auto startTime = std::chrono::steady_clock::now();

    while (true) {
        int n = ::read(mFileDescriptor, &ch, 1);
        
        if (n == 1) {
            if (ch == '\n') {
                // Remove trailing \r if present
                if (!line.empty() && line.back() == '\r') {
                    line.pop_back();
                }
                return line;
            }
            line += ch;
        } else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
            return "";  // Error
        }

        // Check timeout
        if (timeout > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime
            ).count();
            if (elapsed >= timeout) {
                return "";  // Timeout
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

size_t SerialManager::available() const {
    if (!isConnected() || mFileDescriptor < 0) {
        return 0;
    }

    int bytes = 0;
    ioctl(mFileDescriptor, FIONREAD, &bytes);
    return static_cast<size_t>(bytes);
}

// ============================================================================
// Callbacks
// ============================================================================

void SerialManager::setDataCallback(SerialDataCallback callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mDataCallback = callback;
}

void SerialManager::setStateCallback(SerialStateCallback callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mStateCallback = callback;
}

// ============================================================================
// Port Configuration
// ============================================================================

std::string SerialManager::getPortName() const {
    return mPortName;
}

uint32_t SerialManager::getBaudRate() const {
    return mBaudRate;
}

void SerialManager::flush() {
    if (isConnected() && mFileDescriptor >= 0) {
        tcflush(mFileDescriptor, TCIOFLUSH);
    }
}

} // namespace Controller
