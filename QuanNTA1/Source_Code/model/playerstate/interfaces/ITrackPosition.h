/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/interfaces/ITrackPosition.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for track position management.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef ITRACKPOSITION_H
#define ITRACKPOSITION_H

#include <cstdint>

namespace Model {

/**
 * @brief Interface for track position management.
 * 
 * This interface provides access to the current playback position.
 * It follows the Interface Segregation Principle by separating
 * position concerns from playback, volume, and navigation.
 */
class ITrackPosition {
public:
    virtual ~ITrackPosition() = default;

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
    
    /**
     * @brief Get playback version (increments on repeat restart).
     * @return Current version number
     */
    virtual uint32_t getPlaybackVersion() const = 0;
    
    /**
     * @brief Increment playback version (call on repeat restart).
     */
    virtual void incrementPlaybackVersion() = 0;
};

} // namespace Model

#endif // ITRACKPOSITION_H
