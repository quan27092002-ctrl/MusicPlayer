/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/IAppController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for the main application controller.
 */

#ifndef IAPPCONTROLLER_H
#define IAPPCONTROLLER_H

#include <string>
#include <functional>

namespace Controller {

/**
 * @brief Application state enumeration.
 */
enum class AppState {
    UNINITIALIZED = 0,  ///< Not yet initialized
    READY = 1,          ///< Initialized and ready
    RUNNING = 2,        ///< Running (connected to board)
    ERROR = 3           ///< Error state
};

/**
 * @brief Callback for application state changes.
 */
using AppStateCallback = std::function<void(AppState state)>;

/**
 * @brief Abstract interface for the main application controller.
 * 
 * Coordinates all subsystems: Audio, Serial, and PlayerState.
 * Processes commands from S32K board and controls playback.
 */
class IAppController {
public:
    virtual ~IAppController() = default;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Initialize all subsystems.
     * @return true if all subsystems initialized successfully
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown all subsystems.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get current application state.
     * @return Current AppState
     */
    virtual AppState getState() const = 0;

    // ========================================================================
    // Serial Connection
    // ========================================================================

    /**
     * @brief Connect to S32K board via serial port.
     * @param portName Serial port name (e.g., "/dev/ttyUSB0")
     * @param baudRate Baud rate (default 115200)
     * @return true if connection successful
     */
    virtual bool connectToBoard(const std::string& portName, uint32_t baudRate = 115200) = 0;

    /**
     * @brief Disconnect from the board.
     */
    virtual void disconnectFromBoard() = 0;

    /**
     * @brief Check if connected to board.
     * @return true if connected
     */
    virtual bool isConnectedToBoard() const = 0;

    // ========================================================================
    // Playback Control
    // ========================================================================

    /**
     * @brief Load an audio file.
     * @param filePath Path to audio file
     * @return true if loaded successfully
     */
    virtual bool loadTrack(const std::string& filePath) = 0;

    /**
     * @brief Start or resume playback.
     */
    virtual void play() = 0;

    /**
     * @brief Pause playback.
     */
    virtual void pause() = 0;

    /**
     * @brief Stop playback.
     */
    virtual void stop() = 0;

    /**
     * @brief Skip to next track.
     */
    virtual void next() = 0;

    /**
     * @brief Go to previous track.
     */
    virtual void previous() = 0;

    /**
     * @brief Seek to position in current track.
     * @param positionMs Position in milliseconds
     */
    virtual void seek(uint32_t positionMs) = 0;

    // ========================================================================
    // Volume Control
    // ========================================================================

    /**
     * @brief Set volume level.
     * @param volume Volume level (0-100)
     */
    virtual void setVolume(int volume) = 0;

    /**
     * @brief Get current volume level.
     * @return Volume level (0-100)
     */
    virtual int getVolume() const = 0;

    /**
     * @brief Toggle mute state.
     */
    virtual void toggleMute() = 0;

    // ========================================================================
    // Playlist Management
    // ========================================================================

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

    // ========================================================================
    // Callbacks
    // ========================================================================

    /**
     * @brief Set callback for application state changes.
     * @param callback Function to call on state change
     */
    virtual void setStateCallback(AppStateCallback callback) = 0;
};

} // namespace Controller

#endif // IAPPCONTROLLER_H
