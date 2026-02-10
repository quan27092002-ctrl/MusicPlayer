/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/SerialManager.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of SerialManager using composition.
 *              Facade delegating to composed components.
 */

#include "SerialManager.h"

namespace Controller {

// ============================================================================
// Constructor / Destructor
// ============================================================================

SerialManager::SerialManager() {
    mConnection = std::make_unique<SerialConnectionImpl>();
    mIO = std::make_unique<SerialIOImpl>(mConnection.get());
}

SerialManager::~SerialManager() {
    disconnect();
}

// ============================================================================
// ISerialConnection Delegation
// ============================================================================

bool SerialManager::connect(const std::string& portName, uint32_t baudRate) {
    bool result = mConnection->connect(portName, baudRate);
    if (result) {
        mIO->startReadThread();
    }
    return result;
}

void SerialManager::disconnect() {
    mIO->stopReadThread();
    mConnection->disconnect();
}

bool SerialManager::isConnected() const {
    return mConnection->isConnected();
}

SerialState SerialManager::getState() const {
    return mConnection->getState();
}

void SerialManager::setStateCallback(SerialStateCallback callback) {
    mConnection->setStateCallback(callback);
}

std::string SerialManager::getPortName() const {
    return mConnection->getPortName();
}

uint32_t SerialManager::getBaudRate() const {
    return mConnection->getBaudRate();
}

std::vector<std::string> SerialManager::getAvailablePorts() const {
    return mConnection->getAvailablePorts();
}

// ============================================================================
// ISerialIO Delegation
// ============================================================================

int SerialManager::send(const std::string& data) {
    return mIO->send(data);
}

int SerialManager::sendBytes(const uint8_t* data, size_t length) {
    return mIO->sendBytes(data, length);
}

int SerialManager::read(uint8_t* buffer, size_t maxLength) {
    return mIO->read(buffer, maxLength);
}

std::string SerialManager::readLine(uint32_t timeoutMs) {
    return mIO->readLine(timeoutMs);
}

size_t SerialManager::available() const {
    return mIO->available();
}

void SerialManager::flush() {
    mIO->flush();
}

void SerialManager::setDataCallback(SerialDataCallback callback) {
    mIO->setDataCallback(callback);
}

} // namespace Controller
