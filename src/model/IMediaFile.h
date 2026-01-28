/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/IMediaFile.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for MediaFile - represents a single audio file.
 */

#ifndef IMEDIAFILE_H
#define IMEDIAFILE_H

#include <string>
#include <cstdint>

namespace Model {

/**
 * @brief Abstract interface for a media file.
 * 
 * Defines the contract for accessing media file metadata.
 * This allows mocking in unit tests.
 */
class IMediaFile {
public:
    virtual ~IMediaFile() = default;

    /**
     * @brief Get the filename (without path).
     * @return Filename string (e.g., "song.mp3")
     */
    virtual std::string getFilename() const = 0;

    /**
     * @brief Get the full file path.
     * @return Full path string (e.g., "/home/user/music/song.mp3")
     */
    virtual std::string getPath() const = 0;

    /**
     * @brief Get the duration in seconds.
     * @return Duration in seconds, 0 if unknown
     */
    virtual uint32_t getDuration() const = 0;

    /**
     * @brief Check if this media file is valid.
     * @return true if filename and path are not empty
     */
    virtual bool isValid() const = 0;
};

} // namespace Model

#endif // IMEDIAFILE_H
