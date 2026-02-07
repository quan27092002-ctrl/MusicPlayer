/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/BoardCommunicator.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of BoardCommunicatorImpl.
 */

#include "BoardCommunicator.h"
#include <sstream>
#include <algorithm>
#include "../../utils/Logger.h"

namespace Controller {

BoardCommunicatorImpl::BoardCommunicatorImpl(
    std::shared_ptr<ISerialManager> serialManager,
    std::shared_ptr<Model::IPlayerState> playerState
)
    : mSerialManager(serialManager)
    , mPlayerState(playerState)
    , mBoardEventCallback(nullptr)
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

std::vector<std::string> BoardCommunicatorImpl::getAvailablePorts() const {
    if (mSerialManager) {
        return mSerialManager->getAvailablePorts();
    }
    return {};
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

void BoardCommunicatorImpl::setBoardEventCallback(BoardEventCallback callback) {
    // mCommandCallback = nullptr; // Legacy removed
    mBoardEventCallback = callback;
}

void BoardCommunicatorImpl::processCommand(const std::string& rawData) {
    if (!mBoardEventCallback) return;
    
    LOG_INFO("BoardCommunicator received: " << rawData);

    std::string cmd = rawData;
    // Remove trailing newline/cr if present
    cmd.erase(std::remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
    cmd.erase(std::remove(cmd.begin(), cmd.end(), '\r'), cmd.end());
    
    // Parse Protocol
    // 1. RV:<adc_value> (0-4095)
    if (cmd.rfind("RV:", 0) == 0) {
        try {
            int adc = std::stoi(cmd.substr(3));
            int vol = parseVolumeFromADC(adc);
            mBoardEventCallback(BoardEvent::SET_VOLUME, vol);
        } catch (...) {}
        return;
    }
    
    // 2. CMD:<action> or cmd:<action>
    bool isCmd = false;
    std::string action;

    if (cmd.rfind("CMD:", 0) == 0) {
        action = cmd.substr(4);
        isCmd = true;
    } else if (cmd.rfind("cmd:", 0) == 0) {
        action = cmd.substr(4);
        isCmd = true;
    }

    if (isCmd) {
        std::transform(action.begin(), action.end(), action.begin(), ::toupper);
        
        if (action == "PLAY") {
            mBoardEventCallback(BoardEvent::PLAY, 0);
        } else if (action == "PAUSE") {
            mBoardEventCallback(BoardEvent::PAUSE, 0);
        } else if (action == "STOP") {
            mBoardEventCallback(BoardEvent::STOP, 0);
        } else if (action == "NEXT") {
            mBoardEventCallback(BoardEvent::NEXT, 0);
        } else if (action == "PREV") {
            mBoardEventCallback(BoardEvent::PREV, 0);
        }
        return;
    }
}

int BoardCommunicatorImpl::parseVolumeFromADC(int adcValue) {
    // Clamp ADC to 0-4095
    if (adcValue < 0) adcValue = 0;
    if (adcValue > 4095) adcValue = 4095;
    
    // Scale 0-4095 to 0-100
    // vol = (adc * 100) / 4095
    return (adcValue * 100) / 4095;
}

void BoardCommunicatorImpl::setCurrentTrackIndexGetter(std::function<int()> getter) {
    mGetCurrentTrackIndex = getter;
}

} // namespace Controller
