/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/interfaces/IPlaylistManager.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for playlist management operations.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IPLAYLISTMANAGER_H
#define IPLAYLISTMANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <cstdint>
#include <functional>
#include "MediaFile.h"

namespace Controller {

/**
 * @brief Interface for playlist management operations.
 * 
 * This interface provides access to playlist operations such as
 * adding, clearing, loading tracks, and accessing track metadata.
 * It follows the Interface Segregation Principle.
 */
class IPlaylistManager {
public:
    virtual ~IPlaylistManager() = default;

    /**
     * @brief Add track to playlist.
     * @param filePath Path to audio file
     */
    virtual void addToPlaylist(const std::string& filePath) = 0;

    /**
     * @brief Clear the playlist.
     */
    virtual void clearPlaylist() = 0;

    /**
     * @brief Get number of tracks in playlist.
     * @return Track count
     */
    virtual size_t getPlaylistSize() const = 0;

    /**
     * @brief Load all audio files from a directory into playlist.
     * @param directoryPath Path to directory containing audio files
     * @return Number of files loaded
     */
    virtual size_t loadDirectory(const std::string& directoryPath) = 0;

    /**
     * @brief Get track name at index.
     * @param index Track index
     * @return Track name or empty string if invalid index
     */
    virtual std::string getTrackName(size_t index) const = 0;

    /**
     * @brief Get track path at index.
     * @param index Track index
     * @return Track path or empty string if invalid index
     */
    virtual std::string getTrackPath(size_t index) const = 0;

    /**
     * @brief Get track artist at index.
     * @param index Track index
     * @return Track artist or "Unknown Artist" if invalid
     */
    virtual std::string getTrackArtist(size_t index) const = 0;

    /**
     * @brief Get track album at index.
     * @param index Track index
     * @return Track album or "Unknown Album" if invalid
     */
    virtual std::string getTrackAlbum(size_t index) const = 0;

    /**
     * @brief Get track duration at index.
     * @param index Track index
     * @return Duration in seconds or 0 if invalid
     */
    virtual uint32_t getTrackDuration(size_t index) const = 0;

    /**
     * @brief Get the track cover art.
     * @param index Playlist index
     * @return Raw image data
     */
    virtual std::vector<uint8_t> getTrackCoverArt(size_t index) const = 0;
    /**
     * @brief Gets or creates a media file pointer for a given path.
     * @param filePath Absolute path to the file.
     * @return Shared pointer to MediaFile, or nullptr if invalid.
     */
    virtual std::shared_ptr<Model::MediaFile> acquireMediaFile(const std::string& filePath) = 0;

    // Library Accessors
    virtual size_t getLibrarySize() const = 0;
    virtual std::string getLibraryTrackName(size_t index) const = 0;
    virtual std::string getLibraryTrackPath(size_t index) const = 0;
    virtual std::string getLibraryTrackArtist(size_t index) const = 0;
    virtual std::string getLibraryTrackAlbum(size_t index) const = 0;
    virtual std::vector<uint8_t> getLibraryTrackCoverArt(size_t index) const = 0;

    /**
     * @brief Set a callback to be notified when playlist changes.
     */
    virtual void setPlaylistUpdatedCallback(std::function<void()> callback) = 0;
    
    /**
     * @brief Notify listeners that playlist has updated.
     */
    virtual void notifyPlaylistUpdated() = 0;
};

} // namespace Controller

#endif // IPLAYLISTMANAGER_H
