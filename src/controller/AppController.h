/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AppController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Main application controller implementation.
 *              Facade class implementing IAppController using composition.
 *              Delegates to specialized components following SOLID principles.
 */

#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include "IAppController.h"
#include "IAudioPlayer.h"
#include "ISerialManager.h"
#include "../model/IPlayerState.h"
#include "../model/MediaFile.h"

// Include concrete implementations for composition
#include "appcontroller/PlaylistManager.h"
#include "appcontroller/VolumeController.h"
#include "appcontroller/HistoryManager.h"
#include "appcontroller/BoardCommunicator.h"
#include "appcontroller/PlaybackController.h"

#include <memory>
#include <atomic>
#include <memory>
#include <atomic>
#include <mutex>
#include <vector>

namespace Controller {

/**
 * @brief Main application controller (Facade).
 * 
 * Coordinates all subsystems using composition:
 * - PlaylistManagerImpl: Playlist operations
 * - VolumeControllerImpl: Volume control
 * - HistoryManagerImpl: History navigation
 * - BoardCommunicatorImpl: S32K board communication
 * - PlaybackControllerImpl: Playback operations
 * 
 * This facade provides a unified interface while delegating
 * to specialized components, adhering to SRP.
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
    // IAppLifecycle Interface
    // ========================================================================
    bool initialize() override;
    void shutdown() override;
    AppState getState() const override;
    void setStateCallback(AppStateCallback callback) override;

    // ========================================================================
    // IBoardCommunicator Interface (delegates to mBoardCommunicator)
    // ========================================================================
    bool connectToBoard(const std::string& portName, uint32_t baudRate = 115200) override;
    void disconnectFromBoard() override;
    bool isConnectedToBoard() const override;

    /**
     * @brief Get a list of available serial ports.
     * @return Vector of port names.
     */
    std::vector<std::string> getAvailablePorts() const override;

    // ========================================================================
    // IPlaybackController Interface (delegates to mPlaybackController)
    // ========================================================================
    bool loadTrack(const std::string& filePath) override;
    void play() override;
    void pause() override;
    void stop() override;
    void next() override;
    void previous() override;
    void playTrack(int index) override;
    void playPlaylist(const std::vector<std::string>& filePaths) override;
    void queuePlaylist(const std::vector<std::string>& filePaths) override;
    void replaceQueue(const std::vector<std::string>& filePaths) override;
    void queueNext(const std::string& filePath) override;
    void seek(uint32_t positionMs) override;

    // ========================================================================
    // IVolumeController Interface (delegates to mVolumeController)
    // ========================================================================
    void setVolume(int volume) override;
    int getVolume() const override;
    void toggleMute() override;

    // ========================================================================
    // IPlaylistManager Interface (delegates to mPlaylistManager)
    // ========================================================================
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
    std::shared_ptr<Model::MediaFile> acquireMediaFile(const std::string& filePath) override;
    
    // Library Accessors
    size_t getLibrarySize() const override;
    std::string getLibraryTrackName(size_t index) const override;
    std::string getLibraryTrackPath(size_t index) const override;
    std::string getLibraryTrackArtist(size_t index) const override;
    std::string getLibraryTrackAlbum(size_t index) const override;
    std::vector<uint8_t> getLibraryTrackCoverArt(size_t index) const override;
    
    void setPlaylistUpdatedCallback(std::function<void()> callback) override;
    void notifyPlaylistUpdated() override;

    // History Accessors
    size_t getHistorySize() const override;
    std::string getHistoryTrackName(size_t index) const override;
    std::string getHistoryTrackArtist(size_t index) const override;
    std::string getHistoryTrackAlbum(size_t index) const override;
    std::vector<uint8_t> getHistoryTrackCoverArt(size_t index) const override;
    void playHistoryTrack(size_t index) override;

    // ========================================================================
    // IHistoryManager Interface (delegates to mHistoryManager)
    // ========================================================================
    std::vector<int> getHistory() const override;
    std::shared_ptr<Model::MediaFile> getHistoryItem(size_t index) const override;

private:
    // ========================================================================
    // Injected Dependencies
    // ========================================================================
    std::shared_ptr<IAudioPlayer> mAudioPlayer;
    std::shared_ptr<ISerialManager> mSerialManager;
    std::shared_ptr<Model::IPlayerState> mPlayerState;

    // ========================================================================
    // Composed Components (SOLID - SRP)
    // ========================================================================
    std::unique_ptr<PlaylistManagerImpl> mPlaylistManager;
    std::unique_ptr<VolumeControllerImpl> mVolumeController;
    std::unique_ptr<HistoryManagerImpl> mHistoryManager;
    std::unique_ptr<BoardCommunicatorImpl> mBoardCommunicator;
    std::unique_ptr<PlaybackControllerImpl> mPlaybackController;

    // ========================================================================
    // Lifecycle State
    // ========================================================================
    std::atomic<AppState> mAppState;
    AppStateCallback mStateCallback;
    mutable std::mutex mCallbackMutex;

    // ========================================================================
    // Private Helpers
    // ========================================================================
    void notifyStateChange(AppState newState);
    void onSerialDataReceived(const std::string& data);
    void onSerialStateChanged(SerialState state);
    void onAudioStateChanged(AudioState state, uint32_t position);
    void onBoardEventReceived(BoardEvent event, int value);
    
    // Legacy support removed (processCommand)
    // void processCommand(const std::string& command);
};

} // namespace Controller

#endif // APPCONTROLLER_H
