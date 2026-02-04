/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioLifecycleImpl.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IAudioLifecycle.
 */

#ifndef AUDIOLIFECYCLE_IMPL_H
#define AUDIOLIFECYCLE_IMPL_H

#include "interfaces/IAudioLifecycle.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <atomic>
#include <mutex>
#include <functional>

namespace Controller {

/**
 * @brief Concrete implementation of IAudioLifecycle.
 *
 * Manages SDL audio initialization and shutdown.
 */
class AudioLifecycleImpl : public IAudioLifecycle {
public:
    AudioLifecycleImpl();
    ~AudioLifecycleImpl() override;

    // IAudioLifecycle interface
    bool initialize() override;
    void shutdown() override;
    AudioState getState() const override;
    void setCallback(AudioCallback callback) override;
    
    // Additional methods
    bool isInitialized() const;
    void notifyCallback(AudioState state, uint32_t positionMs);
    void setState(AudioState state);
    std::mutex& getMutex();
    AudioCallback getCallback() const;

private:
    std::atomic<AudioState> mState;
    std::atomic<bool> mInitialized;
    AudioCallback mCallback;
    mutable std::mutex mMutex;
};

} // namespace Controller

#endif // AUDIOLIFECYCLE_IMPL_H
