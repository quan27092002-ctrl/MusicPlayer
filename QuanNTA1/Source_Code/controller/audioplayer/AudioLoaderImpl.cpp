/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioLoaderImpl.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of AudioLoaderImpl.
 */

#include "AudioLoaderImpl.h"

namespace Controller {

AudioLoaderImpl::AudioLoaderImpl(AudioLifecycleImpl* lifecycle)
    : mLifecycle(lifecycle)
    , mMusic(nullptr)
    , mDuration(0)
{}

AudioLoaderImpl::~AudioLoaderImpl() {
    unload();
}

bool AudioLoaderImpl::load(const std::string& filePath) {
    if (!mLifecycle || !mLifecycle->isInitialized()) {
        return false;
    }

    unload();

    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        mMusic = Mix_LoadMUS(filePath.c_str());
        if (!mMusic) {
            mLifecycle->setState(AudioState::ERROR);
            return false;
        }

        mCurrentFilePath = filePath;
        mLifecycle->setState(AudioState::LOADED);
        mDuration.store(0);
    }

    mLifecycle->notifyCallback(AudioState::LOADED, 0);
    return true;
}

void AudioLoaderImpl::unload() {
    // Stop playback first
    Mix_HaltMusic();
    
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mMusic) {
        Mix_FreeMusic(mMusic);
        mMusic = nullptr;
    }
    
    mCurrentFilePath.clear();
    mDuration.store(0);
    
    if (mLifecycle) {
        mLifecycle->setState(AudioState::IDLE);
    }
}

bool AudioLoaderImpl::isLoaded() const {
    if (!mLifecycle) return false;
    AudioState state = mLifecycle->getState();
    return state == AudioState::LOADED || 
           state == AudioState::PLAYING || 
           state == AudioState::PAUSED;
}

Mix_Music* AudioLoaderImpl::getMusic() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mMusic;
}

std::string AudioLoaderImpl::getCurrentFilePath() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mCurrentFilePath;
}

uint32_t AudioLoaderImpl::getDuration() const {
    return mDuration.load();
}

} // namespace Controller
