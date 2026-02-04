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

namespace Controller {

// Forward declaration for command callback
using CommandCallback = std::function<void(const std::string&)>;

/**
 * @brief Concrete implementation of IBoardCommunicator.
 * 
 * Manages communication with S32K board via serial port.
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
    
    // Additional methods
    void sendStatusToBoard();
    void setCommandCallback(CommandCallback callback);
    void processCommand(const std::string& command);
    
    // Get current track index callback
    void setCurrentTrackIndexGetter(std::function<int()> getter);
    
private:
    std::shared_ptr<ISerialManager> mSerialManager;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    CommandCallback mCommandCallback;
    std::function<int()> mGetCurrentTrackIndex;
};

} // namespace Controller

#endif // BOARDCOMMUNICATOR_IMPL_H
