/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/PlaybackController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IPlaybackController.
 *              Follows Single Responsibility Principle (SRP).
 */

#ifndef PLAYBACKCONTROLLER_IMPL_H
#define PLAYBACKCONTROLLER_IMPL_H

#include "interfaces/IPlaybackController.h"
#include "PlaylistManager.h"
#include "HistoryManager.h"
#include "../IAudioPlayer.h"
#include "../../model/IPlayerState.h"
#include <memory>
#include <list>
#include <mutex>
#include <functional>

namespace Controller {

/**
 * @brief Concrete implementation of IPlaybackController.
 * 
 * Manages playback operations (play, pause, stop, next, previous, seek).
 */
class PlaybackControllerImpl : public IPlaybackController {
public:
    using MediaFilePtr = std::shared_ptr<Model::MediaFile>;
    
    PlaybackControllerImpl(
        std::shared_ptr<IAudioPlayer> audioPlayer,
        std::shared_ptr<Model::IPlayerState> playerState,
        PlaylistManagerImpl* playlistManager,
        HistoryManagerImpl* historyManager
    );
    ~PlaybackControllerImpl() override = default;
    
    // IPlaybackController interface
    bool loadTrack(const std::string& filePath) override;
    void play() override;
    void pause() override;
    void stop() override;
    void next() override;
    void previous() override;
    void playTrack(int index) override;
    void seek(uint32_t positionMs) override;
    void queueNext(const std::string& filePath) override;
    void replaceQueue(const std::vector<std::string>& filePaths) override;
    void queuePlaylist(const std::vector<std::string>& filePaths) override;
    void playLibrary(int startIndex) override;
    
    // Additional methods
    int getCurrentTrackIndex() const;
    void setStatusCallback(std::function<void()> callback);
    
    // Shuffle & Repeat
    void toggleShuffle();
    void toggleRepeat();
    
    // Access current track iterator
    typename std::list<MediaFilePtr>::iterator& getCurrentIterator();
    
private:
    std::shared_ptr<IAudioPlayer> mAudioPlayer;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    PlaylistManagerImpl* mPlaylistManager;
    HistoryManagerImpl* mHistoryManager;
    
    typename std::list<MediaFilePtr>::iterator mCurrentTrackIterator;
    std::string mCurrentLoadedPath;
    std::function<void()> mStatusCallback;
    
    // For shuffle restore
    std::vector<MediaFilePtr> mOriginalOrder;
    
    int getCurrentTrackIndexLocked() const;
    typename std::list<MediaFilePtr>::iterator getTrackIterator(int index);
};

} // namespace Controller

#endif // PLAYBACKCONTROLLER_IMPL_H
