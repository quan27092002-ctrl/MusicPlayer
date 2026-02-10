/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/interfaces/IPlaylistNavigation.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for playlist navigation (track index, repeat, shuffle).
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IPLAYLISTNAVIGATION_H
#define IPLAYLISTNAVIGATION_H

namespace Model {

/**
 * @brief Enum representing the repeat/loop mode.
 */
enum class RepeatMode {
    NONE = 0,       ///< No repeat
    ONE = 1,        ///< Repeat current track
    ALL = 2         ///< Repeat entire playlist
};

/**
 * @brief Interface for playlist navigation control.
 * 
 * This interface provides access to track index, repeat mode, and shuffle.
 * It follows the Interface Segregation Principle by separating
 * navigation concerns from playback, volume, and position.
 */
class IPlaylistNavigation {
public:
    virtual ~IPlaylistNavigation() = default;

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
     * @brief Cycle through repeat modes (NONE -> ONE -> ALL -> NONE).
     * @return New repeat mode after cycle
     */
    virtual RepeatMode cycleRepeatMode() = 0;

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

    /**
     * @brief Toggle shuffle mode.
     * @return New shuffle state after toggle
     */
    virtual bool toggleShuffle() = 0;
};

} // namespace Model

#endif // IPLAYLISTNAVIGATION_H
