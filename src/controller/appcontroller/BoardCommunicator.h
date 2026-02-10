/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/BoardCommunicator.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IBoardCommunicator.
 *              Follows Single Responsibility Principle (SRP).
 */

#ifndef BOARDCOMMUNICATOR_IMPL_H
#define BOARDCOMMUNICATOR_IMPL_H

#include "interfaces/IBoardCommunicator.h"
#include "../ISerialManager.h"
#include "../../model/IPlayerState.h"
#include <memory>
#include <functional>
#include <chrono>

namespace Controller {

// Event types from board
enum class BoardEvent {
    PLAY,
    PAUSE,
    STOP,
    NEXT,
    PREV,
    SET_VOLUME,
    UNKNOWN
};

// Callback for board events: Event Type, Optional Value (e.g. volume)
using BoardEventCallback = std::function<void(BoardEvent, int)>;

/**
 * @brief Concrete implementation of IBoardCommunicator.
 * 
 * Manages communication with S32K board via serial port.
 * Responsible for parsing the S32K protocol (SRP).
 */
class BoardCommunicatorImpl : public IBoardCommunicator {
public:
    BoardCommunicatorImpl(
        std::shared_ptr<ISerialManager> serialManager,
        std::shared_ptr<Model::IPlayerState> playerState
    );
    ~BoardCommunicatorImpl() override = default;
    
    // IBoardCommunicator interface
    bool connectToBoard(const std::string& portName, uint32_t baudRate = 115200) override;
    void disconnectFromBoard() override;
    bool isConnectedToBoard() const override;
    
    /**
     * @brief Get a list of available serial ports.
     * @return Vector of port names.
     */
    std::vector<std::string> getAvailablePorts() const override;
    
    // Additional methods
    void sendStatusToBoard();
    
    /**
     * @brief Set callback for high-level board events.
     * @param callback Function to handle BoardEvents
     */
    void setBoardEventCallback(BoardEventCallback callback);
    
    /**
     * @brief Process raw string data from SerialManager.
     * Parses protocol (RV:..., CMD:...) and triggers events.
     */
    void processCommand(const std::string& rawData);
    
    // Get current track index callback
    void setCurrentTrackIndexGetter(std::function<int()> getter);
    
private:
    std::shared_ptr<ISerialManager> mSerialManager;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    BoardEventCallback mBoardEventCallback;
    std::function<int()> mGetCurrentTrackIndex;
    
    // Helper to parse ADC value
    int parseVolumeFromADC(int adcValue);

    // Debouncing for Volume
    int mLastVolume = -1;
    int mLastVolumeSent = -2;
    std::chrono::steady_clock::time_point mLastVolumeUpdate;
};

} // namespace Controller

#endif // BOARDCOMMUNICATOR_IMPL_H
