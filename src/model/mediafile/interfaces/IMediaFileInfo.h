/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/interfaces/IMediaFileInfo.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for file information (filename, path, validity).
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IMEDIAFILEINFO_H
#define IMEDIAFILEINFO_H

#include <string>

namespace Model {

/**
 * @brief Interface for accessing basic file information.
 * 
 * This interface provides access to file-level properties such as
 * filename and path. It follows the Interface Segregation Principle
 * by separating file info from metadata and cover art concerns.
 */
class IMediaFileInfo {
public:
    virtual ~IMediaFileInfo() = default;

    /**
     * @brief Get the filename (without path).
     * @return Filename string (e.g., "song.mp3")
     */
    virtual std::string getFilename() const = 0;

    /**
     * @brief Set the filename.
     * @param filename New filename
     */
    virtual void setFilename(const std::string& filename) = 0;

    /**
     * @brief Get the full file path.
     * @return Full path string (e.g., "/home/user/music/song.mp3")
     */
    virtual std::string getPath() const = 0;

    /**
     * @brief Set the file path.
     * @param path New path
     */
    virtual void setPath(const std::string& path) = 0;

    /**
     * @brief Check if this media file info is valid.
     * @return true if filename and path are not empty
     */
    virtual bool isValid() const = 0;
};

} // namespace Model

#endif // IMEDIAFILEINFO_H
