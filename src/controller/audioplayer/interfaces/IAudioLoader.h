/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/interfaces/IAudioLoader.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for audio file loading operations.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IAUDIOLOADER_H
#define IAUDIOLOADER_H

#include <string>

namespace Controller {

/**
 * @brief Interface for audio file loading operations.
 * 
 * Provides access to load, unload, and query loaded state.
 */
class IAudioLoader {
public:
    virtual ~IAudioLoader() = default;

    /**
     * @brief Load an audio file for playback.
     * @param filePath Path to the audio file
     * @return true if file loaded successfully
     */
    virtual bool load(const std::string& filePath) = 0;

    /**
     * @brief Unload the current file and free resources.
     */
    virtual void unload() = 0;

    /**
     * @brief Check if a file is currently loaded.
     * @return true if a file is loaded
     */
    virtual bool isLoaded() const = 0;
};

} // namespace Controller

#endif // IAUDIOLOADER_H
