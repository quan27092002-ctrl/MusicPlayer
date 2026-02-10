/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioVolumeImpl.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of AudioVolumeImpl.
 */

#include "AudioVolumeImpl.h"
#include <algorithm>

namespace Controller {

AudioVolumeImpl::AudioVolumeImpl()
    : mVolume(50)
{}

void AudioVolumeImpl::setVolume(int volume) {
    volume = std::clamp(volume, 0, 100);
    mVolume.store(volume);
    Mix_VolumeMusic(volumeToSDL(volume));
}

int AudioVolumeImpl::getVolume() const {
    return mVolume.load();
}

int AudioVolumeImpl::volumeToSDL(int volume) const {
    return static_cast<int>((volume / 100.0) * MIX_MAX_VOLUME);
}

} // namespace Controller
