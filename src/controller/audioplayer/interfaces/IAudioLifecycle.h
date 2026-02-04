/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/interfaces/IAudioLifecycle.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for audio player lifecycle management.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IAUDIOLIFECYCLE_H
#define IAUDIOLIFECYCLE_H

#include <functional>
#include <cstdint>

namespace Controller {

/**
 * @brief Audio player state.
 */
enum class AudioState {
    IDLE = 0,       ///< No file loaded
    LOADED = 1,     ///< File loaded, ready to play
    PLAYING = 2,    ///< Currently playing
    PAUSED = 3,     ///< Paused
    ERROR = 4,      ///< Error state
    FINISHED = 5    ///< Playback finished
};

/**
 * @brief Callback type for audio events.
 */
using AudioCallback = std::function<void(AudioState state, uint32_t positionMs)>;

/**
 * @brief Interface for audio player lifecycle management.
 * 
 * Provides access to initialization, shutdown, and state management.
 */
class IAudioLifecycle {
public:
    virtual ~IAudioLifecycle() = default;

    /**
     * @brief Initialize the audio player.
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown and cleanup resources.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get the current audio state.
     * @return Current AudioState
     */
    virtual AudioState getState() const = 0;

    /**
     * @brief Set callback for audio state changes.
     * @param callback Function to call on state changes
     */
    virtual void setCallback(AudioCallback callback) = 0;
};

} // namespace Controller

#endif // IAUDIOLIFECYCLE_H
