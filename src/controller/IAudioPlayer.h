/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/IAudioPlayer.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for audio playback functionality.
 */

#ifndef IAUDIOPLAYER_H
#define IAUDIOPLAYER_H

#include <string>
#include <cstdint>
#include <functional>

namespace Controller {

/**
 * @brief Audio player state reported by the player.
 */
enum class AudioState {
    IDLE = 0,       ///< No file loaded
    LOADED = 1,     ///< File loaded, ready to play
    PLAYING = 2,    ///< Currently playing
    PAUSED = 3,     ///< Paused
    ERROR = 4       ///< Error state
};

/**
 * @brief Callback type for audio events.
 * @param state Current audio state
 * @param positionMs Current playback position in milliseconds
 */
using AudioCallback = std::function<void(AudioState state, uint32_t positionMs)>;

/**
 * @brief Abstract interface for audio player.
 * 
 * Defines the contract for audio playback implementations.
 * Implementations may use SDL2, ALSA, or other audio backends.
 */
class IAudioPlayer {
public:
    virtual ~IAudioPlayer() = default;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Initialize the audio player.
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown and cleanup resources.
     */
    virtual void shutdown() = 0;

    // ========================================================================
    // File Operations
    // ========================================================================

    /**
     * @brief Load an audio file for playback.
     * @param filePath Path to the audio file
     * @return true if file loaded successfully
     */
    virtual bool load(const std::string& filePath) = 0;

    /**
     * @brief Unload the current file and free resources.
     */
    virtual void unload() = 0;

    // ========================================================================
    // Playback Control
    // ========================================================================

    /**
     * @brief Start or resume playback.
     */
    virtual void play() = 0;

    /**
     * @brief Pause playback.
     */
    virtual void pause() = 0;

    /**
     * @brief Stop playback and reset position to beginning.
     */
    virtual void stop() = 0;

    /**
     * @brief Seek to a specific position.
     * @param positionMs Position in milliseconds
     */
    virtual void seek(uint32_t positionMs) = 0;

    // ========================================================================
    // Volume Control
    // ========================================================================

    /**
     * @brief Set the playback volume.
     * @param volume Volume level (0-100)
     */
    virtual void setVolume(int volume) = 0;

    /**
     * @brief Get the current volume level.
     * @return Current volume (0-100)
     */
    virtual int getVolume() const = 0;

    // ========================================================================
    // State Queries
    // ========================================================================

    /**
     * @brief Get the current audio state.
     * @return Current AudioState
     */
    virtual AudioState getState() const = 0;

    /**
     * @brief Get the current playback position.
     * @return Position in milliseconds
     */
    virtual uint32_t getPosition() const = 0;

    /**
     * @brief Get the total duration of loaded file.
     * @return Duration in milliseconds, 0 if no file loaded
     */
    virtual uint32_t getDuration() const = 0;

    /**
     * @brief Check if a file is currently loaded.
     * @return true if a file is loaded
     */
    virtual bool isLoaded() const = 0;

    /**
     * @brief Check if currently playing.
     * @return true if playing
     */
    virtual bool isPlaying() const = 0;

    // ========================================================================
    // Callbacks
    // ========================================================================

    /**
     * @brief Set callback for audio state changes.
     * @param callback Function to call on state changes
     */
    virtual void setCallback(AudioCallback callback) = 0;
};

} // namespace Controller

#endif // IAUDIOPLAYER_H
