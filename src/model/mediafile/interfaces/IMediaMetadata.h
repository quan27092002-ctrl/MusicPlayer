/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/interfaces/IMediaMetadata.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for media metadata (artist, album, duration).
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IMEDIAMETADATA_H
#define IMEDIAMETADATA_H

#include <string>
#include <cstdint>

namespace Model {

/**
 * @brief Interface for accessing media metadata.
 * 
 * This interface provides access to metadata properties such as
 * artist, album, and duration. It follows the Interface Segregation
 * Principle by separating metadata from file info and cover art concerns.
 */
class IMediaMetadata {
public:
    virtual ~IMediaMetadata() = default;

    /**
     * @brief Get the duration in seconds.
     * @return Duration in seconds, 0 if unknown
     */
    virtual uint32_t getDuration() const = 0;

    /**
     * @brief Set the duration.
     * @param duration Duration in seconds
     */
    virtual void setDuration(uint32_t duration) = 0;

    /**
     * @brief Get the artist name.
     * @return Artist name string, empty if unknown
     */
    virtual std::string getArtist() const = 0;

    /**
     * @brief Set the artist name.
     * @param artist Artist name
     */
    virtual void setArtist(const std::string& artist) = 0;

    /**
     * @brief Get the album name.
     * @return Album name string, empty if unknown
     */
    virtual std::string getAlbum() const = 0;

    /**
     * @brief Set the album name.
     * @param album Album name
     */
    virtual void setAlbum(const std::string& album) = 0;
};

} // namespace Model

#endif // IMEDIAMETADATA_H
