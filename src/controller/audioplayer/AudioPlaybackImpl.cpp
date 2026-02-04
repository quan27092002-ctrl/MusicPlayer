/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioPlaybackImpl.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of AudioPlaybackImpl.
 */

#include "AudioPlaybackImpl.h"

namespace Controller {

AudioPlaybackImpl* AudioPlaybackImpl::sInstance = nullptr;

static void onMusicFinished() {
    if (AudioPlaybackImpl::sInstance) {
        AudioPlaybackImpl::sInstance->handleMusicFinished();
    }
}

AudioPlaybackImpl::AudioPlaybackImpl(AudioLifecycleImpl* lifecycle, AudioLoaderImpl* loader)
    : mLifecycle(lifecycle)
    , mLoader(loader)
    , mIsManualStop(false)
    , mFinishedCallback(nullptr)
{
    sInstance = this;
    Mix_HookMusicFinished(onMusicFinished);
}

AudioPlaybackImpl::~AudioPlaybackImpl() {
    Mix_HookMusicFinished(nullptr);
    sInstance = nullptr;
}

void AudioPlaybackImpl::play() {
    if (!mLoader || !mLifecycle) return;
    
    Mix_Music* music = mLoader->getMusic();
    if (!music) return;

    AudioState currentState = mLifecycle->getState();
    
    if (currentState == AudioState::PAUSED) {
        Mix_ResumeMusic();
    } else if (currentState == AudioState::LOADED || currentState == AudioState::PLAYING) {
        Mix_PlayMusic(music, 0);
    }

    mLifecycle->setState(AudioState::PLAYING);
    mLifecycle->notifyCallback(AudioState::PLAYING, getPosition());
}

void AudioPlaybackImpl::pause() {
    if (!mLoader || !mLifecycle) return;
    if (mLifecycle->getState() != AudioState::PLAYING) return;

    Mix_PauseMusic();
    mLifecycle->setState(AudioState::PAUSED);
    mLifecycle->notifyCallback(AudioState::PAUSED, getPosition());
}

void AudioPlaybackImpl::stop() {
    if (!mLoader || !mLifecycle) return;
    if (!mLoader->getMusic()) return;

    mIsManualStop.store(true);
    Mix_HaltMusic();
    mIsManualStop.store(false);
    
    if (mLifecycle->getState() != AudioState::IDLE) {
        mLifecycle->setState(AudioState::LOADED);
        mLifecycle->notifyCallback(AudioState::LOADED, 0);
    }
}

void AudioPlaybackImpl::seek(uint32_t positionMs) {
    if (!mLoader || !mLoader->getMusic()) return;
    
    double positionSec = positionMs / 1000.0;
    Mix_SetMusicPosition(positionSec);
}

bool AudioPlaybackImpl::isPlaying() const {
    if (!mLifecycle) return false;
    return mLifecycle->getState() == AudioState::PLAYING && Mix_PlayingMusic();
}

uint32_t AudioPlaybackImpl::getPosition() const {
    return 0; // SDL_mixer 2.6.0+ required for position
}

uint32_t AudioPlaybackImpl::getDuration() const {
    return mLoader ? mLoader->getDuration() : 0;
}

void AudioPlaybackImpl::setFinishedCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mFinishedCallback = callback;
}

void AudioPlaybackImpl::handleMusicFinished() {
    if (mIsManualStop.load()) return;
    
    if (mLifecycle) {
        mLifecycle->setState(AudioState::FINISHED);
        mLifecycle->notifyCallback(AudioState::FINISHED, 0);
    }
    
    std::function<void()> cb;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        cb = mFinishedCallback;
    }
    if (cb) {
        cb();
    }
}

} // namespace Controller
