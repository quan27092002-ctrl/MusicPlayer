/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/PlaylistManager.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation.
 *              Follows Single Responsibility Principle (SRP).
 */

#ifndef PLAYLISTMANAGER_IMPL_H
#define PLAYLISTMANAGER_IMPL_H

#include "interfaces/IPlaylistManager.h"
#include "../../model/MediaFile.h"
#include <vector>
#include <list>
#include <mutex>
#include <memory>
#include <functional>
#include <thread>
#include <atomic>

namespace Controller {

/**
 * @brief Concrete implementation of IPlaylistManager.
 * 
 * Manages the playlist (adding, removing, querying tracks).
 * Thread-safe implementation.
 */
class PlaylistManagerImpl : public IPlaylistManager {
public:
    using MediaFilePtr = std::shared_ptr<Model::MediaFile>;
    using LoadProgressCallback = std::function<void(size_t loaded, size_t total)>;
    
    PlaylistManagerImpl();
    ~PlaylistManagerImpl() override;
    
    // IPlaylistManager interface
    void addToPlaylist(const std::string& filePath) override;
    void clearPlaylist() override;
    size_t getPlaylistSize() const override;
    size_t loadDirectory(const std::string& directoryPath) override;
    std::string getTrackName(size_t index) const override;
    std::string getTrackPath(size_t index) const override;
    std::string getTrackArtist(size_t index) const override;
    std::string getTrackAlbum(size_t index) const override;
    uint32_t getTrackDuration(size_t index) const override;
    std::vector<uint8_t> getTrackCoverArt(size_t index) const override;
    
    // New method
    MediaFilePtr acquireMediaFile(const std::string& filePath) override;

    // Library Accessors
    size_t getLibrarySize() const override;
    std::string getLibraryTrackName(size_t index) const override;
    std::string getLibraryTrackPath(size_t index) const override;
    std::string getLibraryTrackArtist(size_t index) const override;
    std::string getLibraryTrackAlbum(size_t index) const override;
    std::vector<uint8_t> getLibraryTrackCoverArt(size_t index) const override;
    
    // Additional methods for internal use
    MediaFilePtr getTrackAt(size_t index) const;
    std::list<MediaFilePtr>& getPlaylistRef();
    const std::list<MediaFilePtr>& getPlaylistRef() const;
    std::vector<MediaFilePtr>& getMusicLibraryRef();
    std::mutex& getMutex() const;

    // Callback support
    void setPlaylistUpdatedCallback(std::function<void()> callback) override;
    void notifyPlaylistUpdated() override;
    
    // Async loading support
    void loadDirectoryAsync(const std::string& directoryPath, size_t batchSize = 50);
    void stopAsyncLoading();
    bool isLoading() const;
    void setLoadProgressCallback(LoadProgressCallback callback);

private:
    std::vector<MediaFilePtr> mMusicLibrary;
    std::list<MediaFilePtr> mPlaylist;
    mutable std::mutex mMutex;
    std::function<void()> mPlaylistUpdatedCallback;
    
    // Async loading members
    std::thread mLoaderThread;
    std::atomic<bool> mStopLoading{false};
    std::atomic<bool> mIsLoading{false};
    LoadProgressCallback mLoadProgressCallback;
    
    MediaFilePtr findInLibrary(const std::string& filePath) const;
    MediaFilePtr createMediaFile(const std::string& filePath);
};

} // namespace Controller

#endif // PLAYLISTMANAGER_IMPL_H

