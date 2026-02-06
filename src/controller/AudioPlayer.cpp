/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AudioPlayer.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of AudioPlayer using composition.
 *              Facade delegating to composed components.
 */

#include "AudioPlayer.h"

namespace Controller {

// ============================================================================
// Constructor / Destructor
// ============================================================================

AudioPlayer::AudioPlayer() {
    mLifecycle = std::make_unique<AudioLifecycleImpl>();
    mLoader = std::make_unique<AudioLoaderImpl>(mLifecycle.get());
    mPlayback = std::make_unique<AudioPlaybackImpl>(mLifecycle.get(), mLoader.get());
    mVolume = std::make_unique<AudioVolumeImpl>();
}

AudioPlayer::~AudioPlayer() {
    shutdown();
}

// ============================================================================
// IAudioLifecycle Delegation
// ============================================================================

bool AudioPlayer::initialize() {
    return mLifecycle->initialize();
}

void AudioPlayer::shutdown() {
    // Clear playback first
    if (mPlayback) {
        mPlayback->stop();
    }
    
    // Unload music
    if (mLoader) {
        mLoader->unload();
    }
    
    // Shutdown lifecycle
    if (mLifecycle) {
        mLifecycle->shutdown();
    }
}

AudioState AudioPlayer::getState() const {
    return mLifecycle->getState();
}

void AudioPlayer::setCallback(AudioCallback callback) {
    mLifecycle->setCallback(callback);
}

// ============================================================================
// IAudioLoader Delegation
// ============================================================================

bool AudioPlayer::load(const std::string& filePath) {
    // Stop playback first so that the manual stop flag is set (in AudioPlaybackImpl).
    // This prevents the 'Finished' callback from firing when the loader unloads the previous track,
    // which effectively fixes the "double skip" bug.
    if (mPlayback) {
        mPlayback->stop();
    }
    return mLoader->load(filePath);
}

void AudioPlayer::unload() {
    mPlayback->stop();
    mLoader->unload();
}

bool AudioPlayer::isLoaded() const {
    return mLoader->isLoaded();
}

// ============================================================================
// IAudioPlayback Delegation
// ============================================================================

void AudioPlayer::play() {
    mPlayback->play();
}

void AudioPlayer::pause() {
    mPlayback->pause();
}

void AudioPlayer::stop() {
    mPlayback->stop();
}

void AudioPlayer::seek(uint32_t positionMs) {
    mPlayback->seek(positionMs);
}

bool AudioPlayer::isPlaying() const {
    return mPlayback->isPlaying();
}

uint32_t AudioPlayer::getPosition() const {
    return mPlayback->getPosition();
}

uint32_t AudioPlayer::getDuration() const {
    return mPlayback->getDuration();
}

void AudioPlayer::setFinishedCallback(std::function<void()> callback) {
    mPlayback->setFinishedCallback(callback);
}

// ============================================================================
// IAudioVolume Delegation
// ============================================================================

void AudioPlayer::setVolume(int volume) {
    mVolume->setVolume(volume);
}

int AudioPlayer::getVolume() const {
    return mVolume->getVolume();
}

} // namespace Controller
