/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AudioPlayer.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: SDL2-based implementation of IAudioPlayer using SDL_mixer.
 */

#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include "IAudioPlayer.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <string>
#include <mutex>
#include <atomic>

namespace Controller {

/**
 * @brief SDL2-based audio player implementation.
 * 
 * Uses SDL_mixer for audio playback, supporting MP3, WAV, OGG, FLAC formats.
 * Thread-safe for concurrent access.
 */
class AudioPlayer : public IAudioPlayer {
private:
    Mix_Music* mMusic;                    ///< SDL_mixer music handle
    std::string mCurrentFilePath;         ///< Currently loaded file path
    
    std::atomic<AudioState> mState;       ///< Current state
    std::atomic<int> mVolume;             ///< Volume level (0-100)
    std::atomic<uint32_t> mDuration;      ///< Duration in ms
    
    AudioCallback mCallback;              ///< State change callback
    std::function<void()> mFinishedCallback; ///< Called when song ends
    mutable std::mutex mMutex;            ///< Protects callback and music handle
    
    bool mInitialized;                    ///< SDL initialized flag

public:
    static AudioPlayer* sInstance;        ///< Static instance for SDL callback (public for callback)

    // Callback from SDL thread
    void handleMusicFinished();

    // Helper to notify callback
    void notifyCallback(AudioState state, uint32_t positionMs);

    // Convert volume 0-100 to SDL 0-128
    int volumeToSDL(int volume) const;

public:
    /**
     * @brief Constructor.
     */
    AudioPlayer();

    /**
     * @brief Destructor - ensures cleanup.
     */
    ~AudioPlayer() override;

    // Delete copy (SDL resources are unique)
    AudioPlayer(const AudioPlayer&) = delete;
    AudioPlayer& operator=(const AudioPlayer&) = delete;

    // ========================================================================
    // IAudioPlayer Interface Implementation
    // ========================================================================

    // Lifecycle
    bool initialize() override;
    void shutdown() override;

    // File Operations
    bool load(const std::string& filePath) override;
    void unload() override;

    // Playback Control
    void play() override;
    void pause() override;
    void stop() override;
    void seek(uint32_t positionMs) override;

    // Volume Control
    void setVolume(int volume) override;
    int getVolume() const override;

    // State Queries
    AudioState getState() const override;
    uint32_t getPosition() const override;
    uint32_t getDuration() const override;
    bool isLoaded() const override;
    bool isPlaying() const override;

    // Callbacks
    void setCallback(AudioCallback callback) override;
    
    /**
     * @brief Set callback for when playback finishes.
     * @param callback Function to call when song ends
     */
    void setFinishedCallback(std::function<void()> callback);
};

} // namespace Controller

#endif // AUDIOPLAYER_H
