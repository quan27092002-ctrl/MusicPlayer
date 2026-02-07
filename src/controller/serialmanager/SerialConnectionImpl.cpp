/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/serialmanager/SerialConnectionImpl.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of SerialConnectionImpl.
 */

#include "SerialConnectionImpl.h"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>

namespace Controller {

SerialConnectionImpl::SerialConnectionImpl()
    : mFileDescriptor(-1)
    , mBaudRate(0)
    , mState(SerialState::DISCONNECTED)
    , mStateCallback(nullptr)
{}

SerialConnectionImpl::~SerialConnectionImpl() {
    disconnect();
}

bool SerialConnectionImpl::connect(const std::string& portName, uint32_t baudRate) {
    if (isConnected()) {
        disconnect();
    }

    mState.store(SerialState::CONNECTING);
    notifyStateChange(SerialState::CONNECTING);

    mFileDescriptor = open(portName.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (mFileDescriptor < 0) {
        mState.store(SerialState::ERROR);
        notifyStateChange(SerialState::ERROR);
        return false;
    }

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
    notifyStateChange(SerialState::CONNECTED);
    return true;
}

void SerialConnectionImpl::disconnect() {
    if (!isConnected()) {
        return;
    }

    if (mFileDescriptor >= 0) {
        close(mFileDescriptor);
        mFileDescriptor = -1;
    }

    mPortName.clear();
    mBaudRate = 0;
    mState.store(SerialState::DISCONNECTED);
    notifyStateChange(SerialState::DISCONNECTED);
}

bool SerialConnectionImpl::isConnected() const {
    return mState.load() == SerialState::CONNECTED;
}

SerialState SerialConnectionImpl::getState() const {
    return mState.load();
}

void SerialConnectionImpl::setStateCallback(SerialStateCallback callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mStateCallback = callback;
}

std::string SerialConnectionImpl::getPortName() const {
    return mPortName;
}

uint32_t SerialConnectionImpl::getBaudRate() const {
    return mBaudRate;
}

int SerialConnectionImpl::getFileDescriptor() const {
    return mFileDescriptor;
}

void SerialConnectionImpl::setState(SerialState state) {
    mState.store(state);
}

void SerialConnectionImpl::notifyStateChange(SerialState state) {
    SerialStateCallback cb;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        cb = mStateCallback;
    }
    if (cb) {
        cb(state);
    }
}

bool SerialConnectionImpl::configureBaudRate(int fd, uint32_t baudRate) {
    struct termios tty;
    
    if (tcgetattr(fd, &tty) != 0) {
        return false;
    }

    speed_t speed;
    switch (baudRate) {
        case 9600:   speed = B9600; break;
        case 19200:  speed = B19200; break;
        case 38400:  speed = B38400; break;
        case 57600:  speed = B57600; break;
        case 115200: speed = B115200; break;
        case 230400: speed = B230400; break;
        default:     return false;
    }

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    // 8N1 mode
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~CRTSCTS;
    tty.c_cflag |= CREAD | CLOCAL;

    // Raw mode
    tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    tty.c_oflag &= ~OPOST;

    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        return false;
    }

    return true;
}

std::vector<std::string> SerialConnectionImpl::getAvailablePorts() const {
    std::vector<std::string> ports;
    
    // Scan for ttyACM*
    system("ls /dev/ttyACM* > /tmp/serial_ports 2>/dev/null");
    system("ls /dev/ttyUSB* >> /tmp/serial_ports 2>/dev/null");
    
    FILE* fp = fopen("/tmp/serial_ports", "r");
    if (fp) {
        char path[1024];
        while (fgets(path, sizeof(path), fp) != NULL) {
            std::string p(path);
            // removing newline
            p.erase(std::remove(p.begin(), p.end(), '\n'), p.end());
            if (!p.empty()) {
                ports.push_back(p);
            }
        }
        fclose(fp);
    }
    
    // Fallback if empty (for testing/safety)
    if (ports.empty()) {
        // ports.push_back("/dev/ttyACM0"); 
    }
    
    return ports;
}

} // namespace Controller
