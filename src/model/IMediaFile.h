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
#include <vector>

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
     * @brief Get the artist name.
     * @return Artist name string, empty if unknown
     */
    virtual std::string getArtist() const = 0;

    /**
     * @brief Get the album name.
     * @return Album name string, empty if unknown
     */
    virtual std::string getAlbum() const = 0;

    /**
     * @brief Set the artist name.
     * @param artist Artist name
     */
    virtual void setArtist(const std::string& artist) = 0;

    /**
     * @brief Set the album name.
     * @param album Album name
     */
    virtual void setAlbum(const std::string& album) = 0;
    
    /**
     * @brief Get the cover art data.
     * @return Vector containing raw image data
     */
    virtual const std::vector<uint8_t>& getCoverArt() const = 0;
    
    /**
     * @brief Set the cover art data.
     * @param data Raw image data
     */
    virtual void setCoverArt(const std::vector<uint8_t>& data) = 0;
    
    /**
     * @brief Check if this media file is valid.
     * @return true if filename and path are not empty
     */
    virtual bool isValid() const = 0;
};

} // namespace Model

#endif // IMEDIAFILE_H
