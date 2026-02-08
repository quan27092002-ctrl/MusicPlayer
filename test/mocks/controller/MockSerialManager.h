/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/controller/MockSerialManager.h
 * DESCRIPTION: GoogleMock implementation for ISerialManager interface.
 */

#ifndef MOCK_SERIAL_MANAGER_H
#define MOCK_SERIAL_MANAGER_H

#include <gmock/gmock.h>
#include "controller/ISerialManager.h"

namespace Controller {

class MockSerialManager : public ISerialManager {
public:
    // ========================================================================
    // ISerialConnection
    // ========================================================================
    MOCK_METHOD(bool, connect, (const std::string& portName, uint32_t baudRate), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(SerialState, getState, (), (const, override));
    MOCK_METHOD(std::string, getPortName, (), (const, override));
    MOCK_METHOD(uint32_t, getBaudRate, (), (const, override));
    MOCK_METHOD(void, setStateCallback, (SerialStateCallback callback), (override));
    MOCK_METHOD(std::vector<std::string>, getAvailablePorts, (), (const, override));

    // ========================================================================
    // ISerialIO
    // ========================================================================
    MOCK_METHOD(int, send, (const std::string& data), (override));
    MOCK_METHOD(int, sendBytes, (const uint8_t* data, size_t length), (override));
    MOCK_METHOD(int, read, (uint8_t* buffer, size_t maxLength), (override));
    MOCK_METHOD(std::string, readLine, (uint32_t timeout), (override));
    MOCK_METHOD(size_t, available, (), (const, override));
    MOCK_METHOD(void, flush, (), (override));
    MOCK_METHOD(void, setDataCallback, (SerialDataCallback callback), (override));
};

} // namespace Controller

#endif // MOCK_SERIAL_MANAGER_H
