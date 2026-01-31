/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AudioPlayer.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of AudioPlayer.
 */

#include "AudioPlayer.h"
#include <algorithm>

namespace Controller {

// Static instance for SDL callback
AudioPlayer* AudioPlayer::sInstance = nullptr;

// SDL callback when music finishes
static void onMusicFinished() {
    if (AudioPlayer::sInstance) {
        AudioPlayer::sInstance->handleMusicFinished();
    }
}

// ============================================================================
// Constructor / Destructor
// ============================================================================

AudioPlayer::AudioPlayer()
    : mMusic(nullptr)
    , mCurrentFilePath("")
    , mState(AudioState::IDLE)
    , mVolume(50)
    , mDuration(0)
    , mCallback(nullptr)
    , mFinishedCallback(nullptr)
    , mInitialized(false) {
    sInstance = this;
}

AudioPlayer::~AudioPlayer() {
    shutdown();
}

// ============================================================================
// Private Helpers
// ============================================================================

void AudioPlayer::notifyCallback(AudioState state, uint32_t positionMs) {
    // NOTE: Caller must NOT hold mMutex when calling this
    AudioCallback cb;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        cb = mCallback;
    }
    if (cb) {
        cb(state, positionMs);
    }
}

int AudioPlayer::volumeToSDL(int volume) const {
    // SDL_mixer uses 0-128, our interface uses 0-100
    return static_cast<int>((volume / 100.0) * MIX_MAX_VOLUME);
}

void AudioPlayer::handleMusicFinished() {
    // Notify completion
    // We can't lock mMutex here if we call notifyCallback which also locks
    // But notifyCallback handles its own locking locally for mCallback copy
    
    mState.store(AudioState::FINISHED);
    notifyCallback(AudioState::FINISHED, 0);
}

// ============================================================================
// Lifecycle
// ============================================================================

bool AudioPlayer::initialize() {
    if (mInitialized) {
        return true;  // Already initialized
    }

    // Initialize just the audio subsystem (not full SDL)
    // This allows ImGuiView to manage the main SDL lifecycle
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
        mState.store(AudioState::ERROR);
        return false;
    }

    // Initialize SDL_mixer
    // 44100 Hz, default format, stereo, 2048 byte chunks
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        mState.store(AudioState::ERROR);
        return false;
    }

    Mix_HookMusicFinished(onMusicFinished);

    mInitialized = true;
    mState.store(AudioState::IDLE);
    return true;
}

void AudioPlayer::shutdown() {
    if (!mInitialized) {
        return;
    }

    // Clear callbacks first to prevent access during shutdown
    Mix_HookMusicFinished(nullptr);
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mFinishedCallback = nullptr;
        mCallback = nullptr;
    }
    sInstance = nullptr;

    unload();
    
    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    
    mInitialized = false;
    mState.store(AudioState::IDLE);
}

// ============================================================================
// File Operations
// ============================================================================

bool AudioPlayer::load(const std::string& filePath) {
    if (!mInitialized) {
        return false;
    }

    // Unload previous music
    unload();

    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        mMusic = Mix_LoadMUS(filePath.c_str());
        if (!mMusic) {
            mState.store(AudioState::ERROR);
            return false;
        }

        mCurrentFilePath = filePath;
        mState.store(AudioState::LOADED);
        
        // Note: Duration detection requires SDL_mixer 2.6.0+
        mDuration.store(0);
    }

    notifyCallback(AudioState::LOADED, 0);
    return true;
}

void AudioPlayer::unload() {
    stop();
    
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mMusic) {
        Mix_FreeMusic(mMusic);
        mMusic = nullptr;
    }
    
    mCurrentFilePath.clear();
    mDuration.store(0);
    mState.store(AudioState::IDLE);
}

// ============================================================================
// Playback Control
// ============================================================================

void AudioPlayer::play() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        if (!mMusic) {
            return;
        }

        AudioState currentState = mState.load();
        
        if (currentState == AudioState::PAUSED) {
            Mix_ResumeMusic();
        } else if (currentState == AudioState::LOADED || currentState == AudioState::PLAYING) {
            Mix_PlayMusic(mMusic, 0);
        }

        mState.store(AudioState::PLAYING);
    }
    notifyCallback(AudioState::PLAYING, getPosition());
}

void AudioPlayer::pause() {
    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        if (!mMusic || mState.load() != AudioState::PLAYING) {
            return;
        }

        Mix_PauseMusic();
        mState.store(AudioState::PAUSED);
    }
    notifyCallback(AudioState::PAUSED, getPosition());
}

void AudioPlayer::stop() {
    bool shouldNotify = false;
    {
        std::lock_guard<std::mutex> lock(mMutex);
        
        if (!mMusic) {
            return;
        }

        Mix_HaltMusic();
        
        if (mState.load() != AudioState::IDLE) {
            mState.store(AudioState::LOADED);
            shouldNotify = true;
        }
    }
    if (shouldNotify) {
        notifyCallback(AudioState::LOADED, 0);
    }
}

void AudioPlayer::seek(uint32_t positionMs) {
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (!mMusic) {
        return;
    }

    // Convert ms to seconds for SDL_mixer
    double positionSec = positionMs / 1000.0;
    Mix_SetMusicPosition(positionSec);
}

// ============================================================================
// Volume Control
// ============================================================================

void AudioPlayer::setVolume(int volume) {
    volume = std::clamp(volume, 0, 100);
    mVolume.store(volume);
    
    // Apply to SDL_mixer
    Mix_VolumeMusic(volumeToSDL(volume));
}

int AudioPlayer::getVolume() const {
    return mVolume.load();
}

// ============================================================================
// State Queries
// ============================================================================

AudioState AudioPlayer::getState() const {
    return mState.load();
}

uint32_t AudioPlayer::getPosition() const {
    // Note: Position tracking requires SDL_mixer 2.6.0+
    // For older versions, we cannot get precise position
    // Return 0 as fallback
    return 0;
}

uint32_t AudioPlayer::getDuration() const {
    return mDuration.load();
}

bool AudioPlayer::isLoaded() const {
    AudioState state = mState.load();
    return state == AudioState::LOADED || 
           state == AudioState::PLAYING || 
           state == AudioState::PAUSED;
}

bool AudioPlayer::isPlaying() const {
    return mState.load() == AudioState::PLAYING && Mix_PlayingMusic();
}

// ============================================================================
// Callbacks
// ============================================================================

void AudioPlayer::setCallback(AudioCallback callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mCallback = callback;
}

void AudioPlayer::setFinishedCallback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(mMutex);
    mFinishedCallback = callback;
    
    // Register SDL callback
    Mix_HookMusicFinished([]() {
        if (sInstance && sInstance->mFinishedCallback) {
            sInstance->mFinishedCallback();
        }
    });
}

} // namespace Controller
