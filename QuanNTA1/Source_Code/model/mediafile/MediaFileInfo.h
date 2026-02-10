/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/MediaFileInfo.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of IMediaFileInfo.
 */

#ifndef MEDIAFILEINFO_H
#define MEDIAFILEINFO_H

#include "interfaces/IMediaFileInfo.h"
#include <string>

namespace Model {

/**
 * @brief Concrete implementation of IMediaFileInfo.
 * 
 * Stores basic file information: filename and path.
 * Follows Single Responsibility Principle (SRP).
 */
class MediaFileInfo : public IMediaFileInfo {
private:
    std::string mFilename;  ///< File name without path (e.g., "song.mp3")
    std::string mPath;      ///< Full absolute path to the file

public:
    /**
     * @brief Default constructor - creates an invalid/empty MediaFileInfo.
     */
    MediaFileInfo();

    /**
     * @brief Parameterized constructor.
     * @param filename File name (without path)
     * @param path Full path to file
     */
    MediaFileInfo(const std::string& filename, const std::string& path);

    /**
     * @brief Copy constructor.
     */
    MediaFileInfo(const MediaFileInfo& other) = default;

    /**
     * @brief Move constructor.
     */
    MediaFileInfo(MediaFileInfo&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    MediaFileInfo& operator=(const MediaFileInfo& other) = default;

    /**
     * @brief Move assignment operator.
     */
    MediaFileInfo& operator=(MediaFileInfo&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~MediaFileInfo() override = default;

    // ========================================================================
    // IMediaFileInfo Interface Implementation
    // ========================================================================

    std::string getFilename() const override;
    void setFilename(const std::string& filename) override;
    std::string getPath() const override;
    void setPath(const std::string& path) override;
    bool isValid() const override;

    // ========================================================================
    // Operators
    // ========================================================================

    /**
     * @brief Equality operator - compares by path.
     */
    bool operator==(const MediaFileInfo& other) const;

    /**
     * @brief Inequality operator.
     */
    bool operator!=(const MediaFileInfo& other) const;
};

} // namespace Model

#endif // MEDIAFILEINFO_H
