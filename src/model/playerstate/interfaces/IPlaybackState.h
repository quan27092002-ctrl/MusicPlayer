/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/interfaces/IPlaybackState.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for playback state (playing/paused/stopped).
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IPLAYBACKSTATE_H
#define IPLAYBACKSTATE_H

namespace Model {

/**
 * @brief Enum representing the playback state.
 */
enum class PlaybackStatus {
    STOPPED = 0,    ///< No media loaded or playback stopped
    PLAYING = 1,    ///< Currently playing
    PAUSED = 2      ///< Paused
};

/**
 * @brief Interface for playback state control.
 * 
 * This interface provides access to playback state (playing, paused, stopped).
 * It follows the Interface Segregation Principle by separating playback
 * state from volume, position, and navigation concerns.
 */
class IPlaybackState {
public:
    virtual ~IPlaybackState() = default;

    /**
     * @brief Get the current playback status.
     * @return Current PlaybackStatus (STOPPED, PLAYING, PAUSED)
     */
    virtual PlaybackStatus getPlaybackStatus() const = 0;

    /**
     * @brief Set the playback status.
     * @param status New playback status
     */
    virtual void setPlaybackStatus(PlaybackStatus status) = 0;

    /**
     * @brief Check if currently playing.
     * @return true if playing, false otherwise
     */
    virtual bool isPlaying() const = 0;

    /**
     * @brief Toggle play/pause state.
     * @return New playback status after toggle
     */
    virtual PlaybackStatus togglePlayPause() = 0;
};

} // namespace Model

#endif // IPLAYBACKSTATE_H
