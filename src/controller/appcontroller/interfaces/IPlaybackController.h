/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/interfaces/IPlaybackController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for playback control operations.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IPLAYBACKCONTROLLER_H
#define IPLAYBACKCONTROLLER_H

#include <string>
#include <cstdint>

namespace Controller {

/**
 * @brief Interface for playback control operations.
 * 
 * This interface provides access to playback operations such as
 * play, pause, stop, next, previous, and seek. It follows the
 * Interface Segregation Principle by separating playback from
 * volume, playlist, history, and board communication concerns.
 */
class IPlaybackController {
public:
    virtual ~IPlaybackController() = default;

    /**
     * @brief Load an audio file.
     * @param filePath Path to audio file
     * @return true if loaded successfully
     */
    virtual bool loadTrack(const std::string& filePath) = 0;

    /**
     * @brief Start or resume playback.
     */
    virtual void play() = 0;

    /**
     * @brief Pause playback.
     */
    virtual void pause() = 0;

    /**
     * @brief Stop playback.
     */
    virtual void stop() = 0;

    /**
     * @brief Skip to next track.
     */
    virtual void next() = 0;

    /**
     * @brief Go to previous track.
     */
    virtual void previous() = 0;

    /**
     * @brief Play specific track by index.
     * @param index Track index (0-based)
     */
    virtual void playTrack(int index) = 0;

    /**
     * @brief Seek to position in current track.
     * @param positionMs Position in milliseconds
     */
    virtual void seek(uint32_t positionMs) = 0;
};

} // namespace Controller

#endif // IPLAYBACKCONTROLLER_H
