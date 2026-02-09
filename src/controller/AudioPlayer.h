/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AudioPlayer.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: SDL2-based implementation of IAudioPlayer using SDL_mixer.
 *              Facade class using composition following SOLID principles.
 */

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "IAudioPlayer.h"
#include "audioplayer/AudioLifecycleImpl.h"
#include "audioplayer/AudioLoaderImpl.h"
#include "audioplayer/AudioPlaybackImpl.h"
#include "audioplayer/AudioVolumeImpl.h"
#include <memory>

namespace Controller {

/**
 * @brief SDL2-based audio player implementation (Facade).
 * 
 * Uses composition to delegate to specialized components:
 * - AudioLifecycleImpl: Initialization/shutdown
 * - AudioLoaderImpl: File loading/unloading
 * - AudioPlaybackImpl: Play/pause/stop/seek
 * - AudioVolumeImpl: Volume control
 */
class AudioPlayer : public IAudioPlayer {
public:
    AudioPlayer();
    ~AudioPlayer() override;

    // Delete copy
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // ========================================================================
    // IAudioLifecycle (delegates to mLifecycle)
    // ========================================================================
    bool initialize() override;
    void shutdown() override;
    AudioState getState() const override;
    void setCallback(AudioCallback callback) override;

    // ========================================================================
    // IAudioLoader (delegates to mLoader)
    // ========================================================================
    bool load(const std::string& filePath) override;
    void unload() override;
    bool isLoaded() const override;

    // ========================================================================
    // IAudioPlayback (delegates to mPlayback)
    // ========================================================================
    void play() override;
    void pause() override;
    void stop() override;
    void seek(uint32_t positionMs) override;
    bool isPlaying() const override;
    uint32_t getPosition() const override;
    uint32_t getDuration() const override;
    void setFinishedCallback(std::function<void()> callback) override;

    // ========================================================================
    // IAudioVolume (delegates to mVolume)
    // ========================================================================
    void setVolume(int volume) override;
    int getVolume() const override;

private:
    friend class AudioPlayerCoverageTest;
    
    std::unique_ptr<AudioLifecycleImpl> mLifecycle;
    std::unique_ptr<AudioLoaderImpl> mLoader;
    std::unique_ptr<AudioPlaybackImpl> mPlayback;
    std::unique_ptr<AudioVolumeImpl> mVolume;
};

} // namespace Controller

#endif // AUDIOPLAYER_H
