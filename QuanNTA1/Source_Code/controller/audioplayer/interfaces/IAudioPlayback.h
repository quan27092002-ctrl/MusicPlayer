/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/interfaces/IAudioPlayback.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for audio playback control.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IAUDIOPLAYBACK_H
#define IAUDIOPLAYBACK_H

#include <cstdint>
#include <functional>

namespace Controller {

/**
 * @brief Interface for audio playback control.
 * 
 * Provides access to play, pause, stop, and seek operations.
 */
class IAudioPlayback {
public:
    virtual ~IAudioPlayback() = default;

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

    /**
     * @brief Check if currently playing.
     * @return true if playing
     */
    virtual bool isPlaying() const = 0;

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
     * @brief Set callback for when playback finishes.
     * @param callback Function to call when finished
     */
    virtual void setFinishedCallback(std::function<void()> callback) = 0;
};

} // namespace Controller

#endif // IAUDIOPLAYBACK_H
