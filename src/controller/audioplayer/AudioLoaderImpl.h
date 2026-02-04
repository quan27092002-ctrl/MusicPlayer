/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioLoaderImpl.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IAudioLoader.
 */

#ifndef AUDIOLOADER_IMPL_H
#define AUDIOLOADER_IMPL_H

#include "interfaces/IAudioLoader.h"
#include "AudioLifecycleImpl.h"
#include <SDL2/SDL_mixer.h>
#include <string>
#include <atomic>
#include <mutex>

namespace Controller {

/**
 * @brief Concrete implementation of IAudioLoader.
 *
 * Manages loading and unloading audio files.
 */
class AudioLoaderImpl : public IAudioLoader {
public:
    AudioLoaderImpl(AudioLifecycleImpl* lifecycle);
    ~AudioLoaderImpl() override;

    // IAudioLoader interface
    bool load(const std::string& filePath) override;
    void unload() override;
    bool isLoaded() const override;
    
    // Additional methods
    Mix_Music* getMusic() const;
    std::string getCurrentFilePath() const;
    uint32_t getDuration() const;

private:
    AudioLifecycleImpl* mLifecycle;
    Mix_Music* mMusic;
    std::string mCurrentFilePath;
    std::atomic<uint32_t> mDuration;
    mutable std::mutex mMutex;
};

} // namespace Controller

#endif // AUDIOLOADER_IMPL_H
