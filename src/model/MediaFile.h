/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/MediaFile.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IMediaFile using composition.
 *              Facade pattern - delegates to smaller components.
 */

#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include "IMediaFile.h"
#include "mediafile/MediaFileInfo.h"
#include "mediafile/MediaMetadata.h"
#include "mediafile/CoverArt.h"
#include <string>
#include <cstdint>
#include <vector>

namespace Model {

/**
 * @brief Concrete implementation of IMediaFile using composition.
 * 
 * This class acts as a facade, delegating to MediaFileInfo, MediaMetadata,
 * and CoverArt components. It provides backward compatibility with the
 * original MediaFile interface while internally following SOLID principles.
 */
class MediaFile : public IMediaFile {
private:
    MediaFileInfo mFileInfo;    ///< File information component
    MediaMetadata mMetadata;    ///< Metadata component
    CoverArt mCoverArt;         ///< Cover art component

public:
    /**
     * @brief Default constructor - creates an invalid/empty MediaFile.
     */
    MediaFile();

    /**
     * @brief Parameterized constructor.
     * @param filename File name (without path)
     * @param path Full path to file
     * @param duration Duration in seconds (default: 0)
     * @param artist Artist name (default: empty)
     * @param album Album name (default: empty)
     * @param coverArt Cover art data (default: empty)
     */
    MediaFile(const std::string& filename, const std::string& path, 
              uint32_t duration = 0, 
              const std::string& artist = "", 
              const std::string& album = "",
              const std::vector<uint8_t>& coverArt = {});

    /**
     * @brief Copy constructor.
     */
    MediaFile(const MediaFile& other) = default;

    /**
     * @brief Move constructor.
     */
    MediaFile(MediaFile&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    MediaFile& operator=(const MediaFile& other) = default;

    /**
     * @brief Move assignment operator.
     */
    MediaFile& operator=(MediaFile&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~MediaFile() override = default;

    // ========================================================================
    // IMediaFileInfo Interface Implementation
    // ========================================================================

    std::string getFilename() const override;
    void setFilename(const std::string& filename) override;
    std::string getPath() const override;
    void setPath(const std::string& path) override;
    bool isValid() const override;

    // ========================================================================
    // IMediaMetadata Interface Implementation
    // ========================================================================

    uint32_t getDuration() const override;
    void setDuration(uint32_t duration) override;
    std::string getArtist() const override;
    void setArtist(const std::string& artist) override;
    std::string getAlbum() const override;
    void setAlbum(const std::string& album) override;

    // ========================================================================
    // ICoverArt Interface Implementation
    // ========================================================================

    const std::vector<uint8_t>& getCoverArt() const override;
    void setCoverArt(const std::vector<uint8_t>& data) override;
    bool hasCoverArt() const override;

    // ========================================================================
    // Component Access (for advanced use)
    // ========================================================================

    /**
     * @brief Get the file info component.
     * @return Reference to MediaFileInfo
     */
    const MediaFileInfo& getFileInfoComponent() const { return mFileInfo; }

    /**
     * @brief Get the metadata component.
     * @return Reference to MediaMetadata
     */
    const MediaMetadata& getMetadataComponent() const { return mMetadata; }

    /**
     * @brief Get the cover art component.
     * @return Reference to CoverArt
     */
    const CoverArt& getCoverArtComponent() const { return mCoverArt; }

    // ========================================================================
    // Operators
    // ========================================================================

    /**
     * @brief Equality operator - compares by path.
     */
    bool operator==(const MediaFile& other) const;

    /**
     * @brief Inequality operator.
     */
    bool operator!=(const MediaFile& other) const;
};

} // namespace Model

#endif // MEDIAFILE_H
