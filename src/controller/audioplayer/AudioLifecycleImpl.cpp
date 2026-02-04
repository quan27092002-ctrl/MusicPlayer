/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioLifecycleImpl.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of AudioLifecycleImpl.
 */

#include "AudioLifecycleImpl.h"

namespace Controller {

AudioLifecycleImpl::AudioLifecycleImpl()
    : mState(AudioState::IDLE)
    , mInitialized(false)
    , mCallback(nullptr)
{}

AudioLifecycleImpl::~AudioLifecycleImpl() {
    shutdown();
}

bool AudioLifecycleImpl::initialize() {
    if (mInitialized.load()) {
        return true;
    }

    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        mState.store(AudioState::ERROR);
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        mState.store(AudioState::ERROR);
        return false;
    }

    mInitialized.store(true);
    mState.store(AudioState::IDLE);
    return true;
}

void AudioLifecycleImpl::shutdown() {
    if (!mInitialized.load()) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mMutex);
        mCallback = nullptr;
    }

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
    mInitialized.store(false);
    mState.store(AudioState::IDLE);
}

AudioState AudioLifecycleImpl::getState() const {
    return mState.load();
}

void AudioLifecycleImpl::setCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mCallback = callback;
}

bool AudioLifecycleImpl::isInitialized() const {
    return mInitialized.load();
}

void AudioLifecycleImpl::notifyCallback(AudioState state, uint32_t positionMs) {
    AudioCallback cb;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        cb = mCallback;
    }
    if (cb) {
        cb(state, positionMs);
    }
}

void AudioLifecycleImpl::setState(AudioState state) {
    mState.store(state);
}

std::mutex& AudioLifecycleImpl::getMutex() {
    return mMutex;
}

AudioCallback AudioLifecycleImpl::getCallback() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mCallback;
}

} // namespace Controller
