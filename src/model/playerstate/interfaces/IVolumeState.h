/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/interfaces/IVolumeState.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for volume control.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IVOLUMESTATE_H
#define IVOLUMESTATE_H

namespace Model {

/**
 * @brief Interface for volume state control.
 * 
 * This interface provides access to volume level and mute state.
 * It follows the Interface Segregation Principle by separating
 * volume concerns from playback, position, and navigation.
 */
class IVolumeState {
public:
    virtual ~IVolumeState() = default;

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

    /**
     * @brief Toggle mute state.
     * @return New mute state after toggle
     */
    virtual bool toggleMute() = 0;
};

} // namespace Model

#endif // IVOLUMESTATE_H
