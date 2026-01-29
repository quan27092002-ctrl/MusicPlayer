/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/IPlayerState.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for PlayerState - represents the current state of the music player.
 */

#ifndef IPLAYERSTATE_H
#define IPLAYERSTATE_H

#include <cstdint>

namespace Model {

/**
 * @brief Enum representing the playback state.
 */
enum class PlaybackState {
    STOPPED = 0,    ///< No media loaded or playback stopped
    PLAYING = 1,    ///< Currently playing
    PAUSED = 2      ///< Paused
};

/**
 * @brief Enum representing the repeat/loop mode.
 */
enum class RepeatMode {
    NONE = 0,       ///< No repeat
    ONE = 1,        ///< Repeat current track
    ALL = 2         ///< Repeat entire playlist
};

/**
 * @brief Abstract interface for player state.
 * 
 * Defines the contract for accessing and modifying player state.
 * This interface is thread-safe - all implementations must ensure
 * thread safety for concurrent access.
 */
class IPlayerState {
public:
    virtual ~IPlayerState() = default;

    // ========================================================================
    // Playback State
    // ========================================================================

    /**
     * @brief Get the current playback state.
     * @return Current PlaybackState (STOPPED, PLAYING, PAUSED)
     */
    virtual PlaybackState getPlaybackState() const = 0;

    /**
     * @brief Set the playback state.
     * @param state New playback state
     */
    virtual void setPlaybackState(PlaybackState state) = 0;

    /**
     * @brief Check if currently playing.
     * @return true if playing, false otherwise
     */
    virtual bool isPlaying() const = 0;

    // ========================================================================
    // Volume Control
    // ========================================================================

    /**
     * @brief Get the current volume level.
     * @return Volume level (0-100)
     */
    virtual int getVolume() const = 0;

    /**
     * @brief Set the volume level.
     * @param volume Volume level (0-100), will be clamped to valid range
     */
    virtual void setVolume(int volume) = 0;

    /**
     * @brief Check if muted.
     * @return true if muted, false otherwise
     */
    virtual bool isMuted() const = 0;

    /**
     * @brief Set mute state.
     * @param muted true to mute, false to unmute
     */
    virtual void setMuted(bool muted) = 0;

    // ========================================================================
    // Track Position
    // ========================================================================

    /**
     * @brief Get current playback position in seconds.
     * @return Current position in seconds
     */
    virtual uint32_t getCurrentPosition() const = 0;

    /**
     * @brief Set current playback position.
     * @param position Position in seconds
     */
    virtual void setCurrentPosition(uint32_t position) = 0;

    // ========================================================================
    // Playlist Navigation
    // ========================================================================

    /**
     * @brief Get the current track index in playlist.
     * @return Current track index (0-based), -1 if no track selected
     */
    virtual int getCurrentTrackIndex() const = 0;

    /**
     * @brief Set the current track index.
     * @param index Track index (0-based)
     */
    virtual void setCurrentTrackIndex(int index) = 0;

    // ========================================================================
    // Playback Modes
    // ========================================================================

    /**
     * @brief Get the current repeat mode.
     * @return Current RepeatMode
     */
    virtual RepeatMode getRepeatMode() const = 0;

    /**
     * @brief Set the repeat mode.
     * @param mode New repeat mode
     */
    virtual void setRepeatMode(RepeatMode mode) = 0;

    /**
     * @brief Check if shuffle mode is enabled.
     * @return true if shuffle is on
     */
    virtual bool isShuffleEnabled() const = 0;

    /**
     * @brief Set shuffle mode.
     * @param enabled true to enable shuffle
     */
    virtual void setShuffleEnabled(bool enabled) = 0;
};

} // namespace Model

#endif // IPLAYERSTATE_H
