/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/MediaFile.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IMediaFile - represents audio file metadata.
 */

#ifndef MEDIAFILE_H
#define MEDIAFILE_H

#include "IMediaFile.h"
#include <string>
#include <cstdint>
#include <vector>

namespace Model {

/**
 * @brief Concrete implementation of IMediaFile.
 * 
 * Stores metadata about an audio file: filename, path, and duration.
 */
class MediaFile : public IMediaFile {
private:
    std::string mFilename;      ///< File name without path (e.g., "song.mp3")
    std::string mPath;          ///< Full absolute path to the file
    uint32_t mDuration;         ///< Duration in seconds (0 if unknown)
    std::string mArtist;        ///< Artist name
    std::string mAlbum;         ///< Album name
    std::vector<uint8_t> mCoverArtData; ///< Raw image data from ID3 tag

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
    // IMediaFile Interface Implementation
    // ========================================================================

    std::string getFilename() const override;
    std::string getPath() const override;
    uint32_t getDuration() const override;
    std::string getArtist() const override;
    std::string getAlbum() const override;
    bool isValid() const override;

    // ========================================================================
    // Additional Methods
    // ========================================================================

    /**
     * @brief Set the filename.
     * @param filename New filename
     */
    void setFilename(const std::string& filename);

    /**
     * @brief Set the file path.
     * @param path New path
     */
    void setPath(const std::string& path);

    /**
     * @brief Set the duration.
     * @param duration Duration in seconds
     */
    void setDuration(uint32_t duration);

    /**
     * @brief Set the artist name.
     * @param artist Artist name
     */
    void setArtist(const std::string& artist) override;

    /**
     * @brief Set the album name.
     * @param album Album name
     */
    void setAlbum(const std::string& album) override;
    
    /**
     * @brief Get the cover art data.
     * @return Raw image data
     */
    const std::vector<uint8_t>& getCoverArt() const override;

    /**
     * @brief Set the cover art data.
     * @param data Raw image data
     */
    void setCoverArt(const std::vector<uint8_t>& data) override;

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
