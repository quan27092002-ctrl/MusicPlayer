/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/interfaces/IVolumeController.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for volume control operations.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IVOLUMECONTROLLER_H
#define IVOLUMECONTROLLER_H

namespace Controller {

/**
 * @brief Interface for volume control operations.
 * 
 * This interface provides access to volume and mute operations.
 * It follows the Interface Segregation Principle by separating
 * volume control from other concerns.
 */
class IVolumeController {
public:
    virtual ~IVolumeController() = default;

    /**
     * @brief Set volume level.
     * @param volume Volume level (0-100)
     */
    virtual void setVolume(int volume) = 0;

    /**
     * @brief Get current volume level.
     * @return Volume level (0-100)
     */
    virtual int getVolume() const = 0;

    /**
     * @brief Toggle mute state.
     */
    virtual void toggleMute() = 0;
};

} // namespace Controller

#endif // IVOLUMECONTROLLER_H
