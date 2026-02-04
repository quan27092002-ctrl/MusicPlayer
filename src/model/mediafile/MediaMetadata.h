/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/MediaMetadata.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IMediaMetadata.
 */

#ifndef MEDIAMETADATA_H
#define MEDIAMETADATA_H

#include "interfaces/IMediaMetadata.h"
#include <string>
#include <cstdint>

namespace Model {

/**
 * @brief Concrete implementation of IMediaMetadata.
 * 
 * Stores media metadata: artist, album, and duration.
 * Follows Single Responsibility Principle (SRP).
 */
class MediaMetadata : public IMediaMetadata {
private:
    uint32_t mDuration;     ///< Duration in seconds (0 if unknown)
    std::string mArtist;    ///< Artist name
    std::string mAlbum;     ///< Album name

public:
    /**
     * @brief Default constructor - creates empty metadata.
     */
    MediaMetadata();

    /**
     * @brief Parameterized constructor.
     * @param duration Duration in seconds
     * @param artist Artist name
     * @param album Album name
     */
    MediaMetadata(uint32_t duration, 
                  const std::string& artist = "", 
                  const std::string& album = "");

    /**
     * @brief Copy constructor.
     */
    MediaMetadata(const MediaMetadata& other) = default;

    /**
     * @brief Move constructor.
     */
    MediaMetadata(MediaMetadata&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    MediaMetadata& operator=(const MediaMetadata& other) = default;

    /**
     * @brief Move assignment operator.
     */
    MediaMetadata& operator=(MediaMetadata&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~MediaMetadata() override = default;

    // ========================================================================
    // IMediaMetadata Interface Implementation
    // ========================================================================

    uint32_t getDuration() const override;
    void setDuration(uint32_t duration) override;
    std::string getArtist() const override;
    void setArtist(const std::string& artist) override;
    std::string getAlbum() const override;
    void setAlbum(const std::string& album) override;
};

} // namespace Model

#endif // MEDIAMETADATA_H
