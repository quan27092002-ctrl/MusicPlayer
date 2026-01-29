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
private:
    // Subsystems (injected via constructor)
    std::shared_ptr<IAudioPlayer> mAudioPlayer;
    std::shared_ptr<ISerialManager> mSerialManager;
    std::shared_ptr<Model::IPlayerState> mPlayerState;

    // Internal state
    std::atomic<AppState> mAppState;
    AppStateCallback mStateCallback;
    mutable std::mutex mCallbackMutex;

    // Playlist
    std::vector<Model::MediaFile> mPlaylist;
    mutable std::mutex mPlaylistMutex;
    int mVolumeBeforeMute;  // Stored volume for mute/unmute

    // Private helpers
    void notifyStateChange(AppState newState);
    void onSerialDataReceived(const std::string& data);
    void onSerialStateChanged(SerialState state);
    void onAudioStateChanged(AudioState state, uint32_t position);
    void processCommand(const std::string& command);
    void sendStatusToBoard();
    int getCurrentTrackIndex() const;

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

    // Callbacks
    void setStateCallback(AppStateCallback callback) override;
};

} // namespace Controller

#endif // APPCONTROLLER_H
