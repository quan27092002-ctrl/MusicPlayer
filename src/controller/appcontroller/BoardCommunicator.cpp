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
    // 1. RV:<adc_value> (0-4095) or VR:<adc_value>
    bool isRv = false;
    if (cmd.rfind("RV:", 0) == 0) isRv = true;
    else if (cmd.rfind("VR:", 0) == 0) isRv = true;

    if (isRv) {
        try {
            std::string valStr = cmd.substr(3);
            LOG_INFO("Parsing RV value: " << valStr);
            int adc = std::stoi(valStr);
            int vol = parseVolumeFromADC(adc);

            
            // Rate Limiting Logic
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - mLastVolumeUpdate).count();
                
            // Only update if:
            // 1. Enough time passed (e.g. 50ms) AND value changed
            // 2. OR Value changed significantly (e.g. > 2 diff) (immediate update)
            // 3. OR First update
            
            bool shouldUpdate = false;
            
            if (mLastVolume == -1) {
                shouldUpdate = true; // First time
            } else if (std::abs(vol - mLastVolume) > 2) {
                shouldUpdate = true; // Significant change (e.g. fast knob turn)
            } else if (mLastVolume != vol && elapsed > 50) {
                shouldUpdate = true; // Timer expired and value changed
            }
            
            if (shouldUpdate) {
                mLastVolume = vol;
                // Only notify if the SENT volume is different (avoid spamming same value)
                if (mLastVolumeSent != vol) {
                    LOG_INFO("Calculated Volume: " << vol);
                    mBoardEventCallback(BoardEvent::SET_VOLUME, vol);
                    mLastVolumeSent = vol;
                    mLastVolumeUpdate = now;
                }
            } else {
                 // LOG_INFO("Skipping volume update - Rate Limited");
            }

        } catch (const std::exception& e) {
            LOG_ERROR("Failed to parse RV: " << e.what());
        } catch (...) {
            LOG_ERROR("Unknown error parsing RV");
        }
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
