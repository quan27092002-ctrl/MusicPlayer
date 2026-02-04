/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/BoardCommunicator.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of BoardCommunicatorImpl.
 */

#include "BoardCommunicator.h"
#include <sstream>
#include <algorithm>

namespace Controller {

BoardCommunicatorImpl::BoardCommunicatorImpl(
    std::shared_ptr<ISerialManager> serialManager,
    std::shared_ptr<Model::IPlayerState> playerState
)
    : mSerialManager(serialManager)
    , mPlayerState(playerState)
    , mCommandCallback(nullptr)
    , mGetCurrentTrackIndex(nullptr)
{}

bool BoardCommunicatorImpl::connectToBoard(const std::string& portName, uint32_t baudRate) {
    if (!mSerialManager) {
        return false;
    }
    return mSerialManager->connect(portName, baudRate);
}

void BoardCommunicatorImpl::disconnectFromBoard() {
    if (mSerialManager) {
        mSerialManager->disconnect();
    }
}

bool BoardCommunicatorImpl::isConnectedToBoard() const {
    return mSerialManager && mSerialManager->isConnected();
}

void BoardCommunicatorImpl::sendStatusToBoard() {
    if (!mSerialManager || !mSerialManager->isConnected()) {
        return;
    }

    std::stringstream ss;
    ss << "STATUS:";
    
    if (mPlayerState) {
        switch (mPlayerState->getPlaybackState()) {
            case Model::PlaybackStatus::PLAYING:
                ss << "PLAYING";
                break;
            case Model::PlaybackStatus::PAUSED:
                ss << "PAUSED";
                break;
            case Model::PlaybackStatus::STOPPED:
                ss << "STOPPED";
                break;
        }
        ss << ",VOL:" << mPlayerState->getVolume();
        ss << ",MUTE:" << (mPlayerState->isMuted() ? "1" : "0");
        
        if (mGetCurrentTrackIndex) {
            ss << ",TRACK:" << mGetCurrentTrackIndex();
        }
    } else {
        ss << "UNKNOWN";
    }
    
    ss << "\n";
    mSerialManager->send(ss.str());
}

void BoardCommunicatorImpl::setCommandCallback(CommandCallback callback) {
    mCommandCallback = callback;
}

void BoardCommunicatorImpl::processCommand(const std::string& command) {
    if (mCommandCallback) {
        mCommandCallback(command);
    }
}

void BoardCommunicatorImpl::setCurrentTrackIndexGetter(std::function<int()> getter) {
    mGetCurrentTrackIndex = getter;
}

} // namespace Controller
