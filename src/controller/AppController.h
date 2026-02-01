/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AppController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Main application controller implementation.
 */

#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include "IAppController.h"
#include "IAudioPlayer.h"
#include "ISerialManager.h"
#include "../model/IPlayerState.h"
#include "../model/MediaFile.h"

#include <memory>
#include <vector>
#include <list>
#include <mutex>
#include <atomic>
#include <string>

namespace Controller {

/**
 * @brief Main application controller.
 * 
 * Coordinates AudioPlayer, SerialManager, and PlayerState.
 * Processes commands from S32K board and manages playback.
 */
class AppController : public IAppController {
public:
    /**
     * @brief Constructor with dependency injection.
     * @param audioPlayer Audio player implementation
     * @param serialManager Serial manager implementation
     * @param playerState Player state model
     */
    AppController(
        std::shared_ptr<IAudioPlayer> audioPlayer,
        std::shared_ptr<ISerialManager> serialManager,
        std::shared_ptr<Model::IPlayerState> playerState
    );

    /**
     * @brief Destructor.
     */
    ~AppController() override;

    // Delete copy
    AppController(const AppController&) = delete;
    AppController& operator=(const AppController&) = delete;

    // ========================================================================
    // IAppController Interface Implementation
    // ========================================================================

    // Lifecycle
    bool initialize() override;
    void shutdown() override;
    AppState getState() const override;

    // Serial Connection
    bool connectToBoard(const std::string& portName, uint32_t baudRate = 115200) override;
    void disconnectFromBoard() override;
    bool isConnectedToBoard() const override;

    // Playback Control
    bool loadTrack(const std::string& filePath) override;
    void play() override;
    void pause() override;
    void stop() override;
    void next() override;
    void previous() override;
    void playTrack(int index) override;
    
    void seek(uint32_t positionMs) override;

    // Volume Control
    void setVolume(int volume) override;
    int getVolume() const override;
    void toggleMute() override;

    // Playlist Management
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
    
    // History
    std::vector<int> getHistory() const override;

    // Callbacks
    void setStateCallback(AppStateCallback callback) override;

private:
    // Subsystems (injected via constructor)
    std::shared_ptr<IAudioPlayer> mAudioPlayer;
    std::shared_ptr<ISerialManager> mSerialManager;
    std::shared_ptr<Model::IPlayerState> mPlayerState;

    // Internal state
    std::atomic<AppState> mAppState;
    AppStateCallback mStateCallback;
    mutable std::mutex mCallbackMutex;

    // Data Structures
    using MediaFilePtr = std::shared_ptr<Model::MediaFile>;
    
    // 1. Music Library (The Master Database)
    std::vector<MediaFilePtr> mMusicLibrary;
    
    // 2. Playlist (The Playback Queue)
    std::list<MediaFilePtr> mPlaylist;
    
    // 3. Playback Position
    // We use iterator for O(1) next/prev, but need fallback if iterator is invalid
    typename std::list<MediaFilePtr>::iterator mCurrentTrackIterator;
    
    mutable std::mutex mPlaylistMutex;
    int mVolumeBeforeMute;

    // History (Recent Songs) - Stores pointers to ensure validity even if playlist changes
    std::vector<MediaFilePtr> mHistoryStack;

    // Private helpers
    int getCurrentTrackIndexLocked() const;
    void notifyStateChange(AppState newState);
    void onSerialDataReceived(const std::string& data);
    void onSerialStateChanged(SerialState state);
    void onAudioStateChanged(AudioState state, uint32_t position);
    void processCommand(const std::string& command);
    void sendStatusToBoard();
    
    // Helper to bridge Index-based API with Iterator-based List
    int getCurrentTrackIndex() const;
    typename std::list<MediaFilePtr>::iterator getTrackIterator(int index);
    typename std::list<MediaFilePtr>::const_iterator getTrackIteratorConst(int index) const;

    void pushHistory(MediaFilePtr track);
    MediaFilePtr popHistory();
};

} // namespace Controller

#endif // APPCONTROLLER_H
