/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/interfaces/IAudioVolume.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for audio volume control.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IAUDIOVOLUME_H
#define IAUDIOVOLUME_H

namespace Controller {

/**
 * @brief Interface for audio volume control.
 * 
 * Provides access to volume get/set operations.
 */
class IAudioVolume {
public:
    virtual ~IAudioVolume() = default;

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
};

} // namespace Controller

#endif // IAUDIOVOLUME_H
