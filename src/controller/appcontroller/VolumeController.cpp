/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/VolumeController.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of VolumeControllerImpl.
 */

#include "VolumeController.h"
#include <algorithm>

namespace Controller {

VolumeControllerImpl::VolumeControllerImpl(
    std::shared_ptr<IAudioPlayer> audioPlayer,
    std::shared_ptr<Model::IPlayerState> playerState
)
    : mAudioPlayer(audioPlayer)
    , mPlayerState(playerState)
    , mVolumeBeforeMute(50) 
{}

void VolumeControllerImpl::setVolume(int volume) {
    volume = std::clamp(volume, 0, 100);
    
    if (mPlayerState) {
        mPlayerState->setVolume(volume);
        
        if (!mPlayerState->isMuted() && mAudioPlayer) {
            mAudioPlayer->setVolume(volume);
        }
    }
}

int VolumeControllerImpl::getVolume() const {
    if (mPlayerState) {
        return mPlayerState->getVolume();
    }
    return 50;
}

void VolumeControllerImpl::toggleMute() {
    if (!mPlayerState || !mAudioPlayer) {
        return;
    }

    bool wasMuted = mPlayerState->isMuted();
    
    if (wasMuted) {
        mPlayerState->setMuted(false);
        mAudioPlayer->setVolume(mPlayerState->getVolume());
    } else {
        mPlayerState->setMuted(true);
        mAudioPlayer->setVolume(0);
    }
}

} // namespace Controller
