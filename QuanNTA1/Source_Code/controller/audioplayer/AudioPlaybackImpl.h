/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioPlaybackImpl.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of IAudioPlayback.
 */

#ifndef AUDIOPLAYBACK_IMPL_H
#define AUDIOPLAYBACK_IMPL_H

#include "interfaces/IAudioPlayback.h"
#include "AudioLifecycleImpl.h"
#include "AudioLoaderImpl.h"
#include <SDL2/SDL_mixer.h>
#include <atomic>
#include <mutex>
#include <functional>

namespace Controller {

/**
 * @brief Concrete implementation of IAudioPlayback.
 *
 * Manages playback operations (play, pause, stop, seek).
 */
class AudioPlaybackImpl : public IAudioPlayback {
public:
    static AudioPlaybackImpl* sInstance;
    
    AudioPlaybackImpl(AudioLifecycleImpl* lifecycle, AudioLoaderImpl* loader);
    ~AudioPlaybackImpl() override;

    // IAudioPlayback interface
    void play() override;
    void pause() override;
    void stop() override;
    void seek(uint32_t positionMs) override;
    bool isPlaying() const override;
    uint32_t getPosition() const override;
    uint32_t getDuration() const override;
    void setFinishedCallback(std::function<void()> callback) override;
    
    // SDL callback handler
    void handleMusicFinished();

private:
    AudioLifecycleImpl* mLifecycle;
    AudioLoaderImpl* mLoader;
    std::atomic<bool> mIsManualStop;
    std::function<void()> mFinishedCallback;
    mutable std::mutex mMutex;
};

} // namespace Controller

#endif // AUDIOPLAYBACK_IMPL_H
