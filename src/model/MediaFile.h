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
     */
    MediaFile(const std::string& filename, const std::string& path, uint32_t duration = 0);

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
