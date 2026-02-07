/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/IAppController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Aggregate interface for the main application controller.
 *              Combines multiple small interfaces for backward compatibility.
 */

#ifndef IAPPCONTROLLER_H
#define IAPPCONTROLLER_H

#include "appcontroller/interfaces/IAppLifecycle.h"
#include "appcontroller/interfaces/IPlaybackController.h"
#include "appcontroller/interfaces/IVolumeController.h"
#include "appcontroller/interfaces/IPlaylistManager.h"
#include "appcontroller/interfaces/IHistoryManager.h"
#include "appcontroller/interfaces/IBoardCommunicator.h"
#include "StorageManager.h"

namespace Controller {

/**
 * @brief Aggregate interface for the main application controller.
 * 
 * Combines IAppLifecycle, IPlaybackController, IVolumeController,
 * IPlaylistManager, IHistoryManager, and IBoardCommunicator interfaces.
 * This provides backward compatibility while following ISP.
 * 
 * Clients that need all features can depend on this interface,
 * while clients needing specific features can depend on smaller interfaces.
 */
class IAppController : public IAppLifecycle,
                       public IPlaybackController,
                       public IVolumeController,
                       public IPlaylistManager,
                       public IHistoryManager,
                       public IBoardCommunicator {
public:
    // Library Accessors (Re-exposed from IPlaylistManager)
    virtual size_t getLibrarySize() const = 0;
    virtual std::string getLibraryTrackName(size_t index) const = 0;
    virtual std::string getLibraryTrackPath(size_t index) const = 0;
    virtual std::string getLibraryTrackArtist(size_t index) const = 0;
    virtual std::string getLibraryTrackAlbum(size_t index) const = 0;
    virtual std::vector<uint8_t> getLibraryTrackCoverArt(size_t index) const = 0;
    
    // Callback for playlist updates
    virtual void setPlaylistUpdatedCallback(std::function<void()> callback) = 0;

    // History Accessors
    virtual size_t getHistorySize() const = 0;
    virtual std::string getHistoryTrackName(size_t index) const = 0;
    virtual std::string getHistoryTrackArtist(size_t index) const = 0;
    virtual std::string getHistoryTrackAlbum(size_t index) const = 0;
    virtual std::vector<uint8_t> getHistoryTrackCoverArt(size_t index) const = 0;
    virtual void playHistoryTrack(size_t index) = 0;

    /**
     * @brief Play a track by index in current queue.
     * @param index Index in the playlist
     */
    virtual void playTrack(int index) = 0;

    /**
     * @brief Play a list of tracks (replaces current queue).
     * @param filePaths Vector of file paths
     */
    virtual void playPlaylist(const std::vector<std::string>& filePaths) = 0;
    
    /**
     * @brief Play library starting from index.
     * @param startIndex Start index in library
     */
    virtual void playLibrary(int startIndex) = 0;
    
    /**
     * @brief Queue a track to play next.
     * @param filePath Path to file
     */
    virtual void queueNext(const std::string& filePath) = 0;

    // Storage Management
    virtual std::vector<StorageDevice> getStorageDevices() = 0;
    virtual size_t loadFromStorage(const std::string& path) = 0;

    virtual ~IAppController() = default;
};

} // namespace Controller

#endif // IAPPCONTROLLER_H
