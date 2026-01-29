/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AppController.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of the main application controller.
 */

#include "AppController.h"
#include <algorithm>
#include <sstream>
#include <filesystem>

namespace Controller {

// ============================================================================
// Constructor / Destructor
// ============================================================================

AppController::AppController(
    std::shared_ptr<IAudioPlayer> audioPlayer,
    std::shared_ptr<ISerialManager> serialManager,
    std::shared_ptr<Model::IPlayerState> playerState
)
    : mAudioPlayer(audioPlayer)
    , mSerialManager(serialManager)
    , mPlayerState(playerState)
    , mAppState(AppState::UNINITIALIZED)
    , mStateCallback(nullptr)
    , mVolumeBeforeMute(50) {
}

AppController::~AppController() {
    shutdown();
}

// ============================================================================
// Private Helpers
// ============================================================================

void AppController::notifyStateChange(AppState newState) {
    mAppState.store(newState);
    
    AppStateCallback cb;
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        cb = mStateCallback;
    }
    if (cb) {
        cb(newState);
    }
}

void AppController::onSerialDataReceived(const std::string& data) {
    processCommand(data);
}

void AppController::onSerialStateChanged(SerialState state) {
    if (state == SerialState::CONNECTED) {
        notifyStateChange(AppState::RUNNING);
        sendStatusToBoard();
    } else if (state == SerialState::DISCONNECTED || state == SerialState::ERROR) {
        if (mAppState.load() == AppState::RUNNING) {
            notifyStateChange(AppState::READY);
        }
    }
}

void AppController::onAudioStateChanged(AudioState state, uint32_t position) {
    (void)position;  // May use later

    if (mPlayerState) {
        switch (state) {
            case AudioState::PLAYING:
                mPlayerState->setPlaybackState(Model::PlaybackState::PLAYING);
                break;
            case AudioState::PAUSED:
                mPlayerState->setPlaybackState(Model::PlaybackState::PAUSED);
                break;
            case AudioState::IDLE:
            case AudioState::LOADED:
                mPlayerState->setPlaybackState(Model::PlaybackState::STOPPED);
                break;
            case AudioState::ERROR:
                mPlayerState->setPlaybackState(Model::PlaybackState::STOPPED);
                break;
        }
    }

    // Notify board of state change
    sendStatusToBoard();
}

void AppController::processCommand(const std::string& command) {
    // Parse command from S32K board
    // Expected formats: PLAY, PAUSE, STOP, NEXT, PREV, VOL:50, MUTE
    
    std::string cmd = command;
    // Convert to uppercase for comparison
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

    if (cmd == "PLAY") {
        play();
    } else if (cmd == "PAUSE") {
        pause();
    } else if (cmd == "STOP") {
        stop();
    } else if (cmd == "NEXT") {
        next();
    } else if (cmd == "PREV" || cmd == "PREVIOUS") {
        previous();
    } else if (cmd == "MUTE") {
        toggleMute();
    } else if (cmd.rfind("VOL:", 0) == 0) {
        // Volume command: VOL:50
        try {
            int vol = std::stoi(cmd.substr(4));
            setVolume(vol);
        } catch (...) {
            // Invalid volume format
        }
    } else if (cmd.rfind("LOAD:", 0) == 0) {
        // Load command: LOAD:/path/to/file.mp3
        std::string path = command.substr(5);  // Use original case for path
        loadTrack(path);
    } else if (cmd == "STATUS") {
        sendStatusToBoard();
    }
}

void AppController::sendStatusToBoard() {
    if (!mSerialManager || !mSerialManager->isConnected()) {
        return;
    }

    // Build status message
    std::stringstream ss;
    ss << "STATUS:";
    
    if (mPlayerState) {
        switch (mPlayerState->getPlaybackState()) {
            case Model::PlaybackState::PLAYING:
                ss << "PLAYING";
                break;
            case Model::PlaybackState::PAUSED:
                ss << "PAUSED";
                break;
            case Model::PlaybackState::STOPPED:
                ss << "STOPPED";
                break;
        }
        ss << ",VOL:" << mPlayerState->getVolume();
        ss << ",MUTE:" << (mPlayerState->isMuted() ? "1" : "0");
        ss << ",TRACK:" << getCurrentTrackIndex();
    } else {
        ss << "UNKNOWN";
    }
    
    ss << "\n";
    mSerialManager->send(ss.str());
}

int AppController::getCurrentTrackIndex() const {
    if (mPlayerState) {
        return mPlayerState->getCurrentTrackIndex();
    }
    return -1;
}

// ============================================================================
// Lifecycle
// ============================================================================

bool AppController::initialize() {
    if (mAppState.load() != AppState::UNINITIALIZED) {
        return true;  // Already initialized
    }

    // Initialize audio player
    if (mAudioPlayer) {
        if (!mAudioPlayer->initialize()) {
            notifyStateChange(AppState::ERROR);
            return false;
        }

        // Set audio callback
        mAudioPlayer->setCallback([this](AudioState state, uint32_t pos) {
            onAudioStateChanged(state, pos);
        });
    }

    // Set serial callbacks (will connect later)
    if (mSerialManager) {
        mSerialManager->setDataCallback([this](const std::string& data) {
            onSerialDataReceived(data);
        });
        mSerialManager->setStateCallback([this](SerialState state) {
            onSerialStateChanged(state);
        });
    }

    // Initialize volume from player state
    if (mPlayerState && mAudioPlayer) {
        mAudioPlayer->setVolume(mPlayerState->getVolume());
    }

    notifyStateChange(AppState::READY);
    return true;
}

void AppController::shutdown() {
    disconnectFromBoard();

    if (mAudioPlayer) {
        mAudioPlayer->shutdown();
    }

    clearPlaylist();
    
    // Clear callbacks before final state change to avoid calling deleted objects
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mStateCallback = nullptr;
    }
    
    mAppState.store(AppState::UNINITIALIZED);
}

AppState AppController::getState() const {
    return mAppState.load();
}

// ============================================================================
// Serial Connection
// ============================================================================

bool AppController::connectToBoard(const std::string& portName, uint32_t baudRate) {
    if (!mSerialManager) {
        return false;
    }

    return mSerialManager->connect(portName, baudRate);
}

void AppController::disconnectFromBoard() {
    if (mSerialManager) {
        mSerialManager->disconnect();
    }
}

bool AppController::isConnectedToBoard() const {
    return mSerialManager && mSerialManager->isConnected();
}

// ============================================================================
// Playback Control
// ============================================================================

bool AppController::loadTrack(const std::string& filePath) {
    if (!mAudioPlayer) {
        return false;
    }

    bool success = mAudioPlayer->load(filePath);
    
    if (success) {
        // Find track in playlist and update index
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        for (size_t i = 0; i < mPlaylist.size(); ++i) {
            if (mPlaylist[i].getPath() == filePath) {
                if (mPlayerState) {
                    mPlayerState->setCurrentTrackIndex(static_cast<int>(i));
                }
                break;
            }
        }
    }

    return success;
}

void AppController::play() {
    if (mAudioPlayer) {
        // If muted, unmute first
        if (mPlayerState && mPlayerState->isMuted()) {
            mPlayerState->setMuted(false);
            mAudioPlayer->setVolume(mPlayerState->getVolume());
        }
        mAudioPlayer->play();
    }
}

void AppController::pause() {
    if (mAudioPlayer) {
        mAudioPlayer->pause();
    }
}

void AppController::stop() {
    if (mAudioPlayer) {
        mAudioPlayer->stop();
    }
}

void AppController::next() {
    std::string pathToLoad;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        
        if (mPlaylist.empty()) {
            return;
        }

        int currentIndex = getCurrentTrackIndex();
        int nextIndex = (currentIndex + 1) % static_cast<int>(mPlaylist.size());
        
        if (nextIndex >= 0 && nextIndex < static_cast<int>(mPlaylist.size())) {
            pathToLoad = mPlaylist[nextIndex].getPath();
        }
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    }
}

void AppController::previous() {
    std::string pathToLoad;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        
        if (mPlaylist.empty()) {
            return;
        }

        int currentIndex = getCurrentTrackIndex();
        int prevIndex = currentIndex - 1;
        if (prevIndex < 0) {
            prevIndex = static_cast<int>(mPlaylist.size()) - 1;
        }
        
        if (prevIndex >= 0 && prevIndex < static_cast<int>(mPlaylist.size())) {
            pathToLoad = mPlaylist[prevIndex].getPath();
        }
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    }
}

// ============================================================================
// Volume Control
// ============================================================================

void AppController::setVolume(int volume) {
    volume = std::clamp(volume, 0, 100);
    
    if (mPlayerState) {
        mPlayerState->setVolume(volume);
        
        // If not muted, apply to audio player
        if (!mPlayerState->isMuted() && mAudioPlayer) {
            mAudioPlayer->setVolume(volume);
        }
    }

    sendStatusToBoard();
}

int AppController::getVolume() const {
    if (mPlayerState) {
        return mPlayerState->getVolume();
    }
    return 50;  // Default
}

void AppController::toggleMute() {
    if (!mPlayerState || !mAudioPlayer) {
        return;
    }

    bool wasMuted = mPlayerState->isMuted();
    
    if (wasMuted) {
        // Unmute - restore previous volume
        mPlayerState->setMuted(false);
        mAudioPlayer->setVolume(mPlayerState->getVolume());
    } else {
        // Mute - set volume to 0
        mPlayerState->setMuted(true);
        mAudioPlayer->setVolume(0);
    }

    sendStatusToBoard();
}

// ============================================================================
// Playlist Management
// ============================================================================

void AppController::addToPlaylist(const std::string& filePath) {
    // Extract filename from path
    std::string filename = filePath;
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filePath.substr(lastSlash + 1);
    }

    Model::MediaFile file(filename, filePath);
    
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    mPlaylist.push_back(file);

    // If this is the first track, set track index to 0
    if (mPlaylist.size() == 1 && mPlayerState) {
        mPlayerState->setCurrentTrackIndex(0);
    }
}

void AppController::clearPlaylist() {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    mPlaylist.clear();
    
    if (mPlayerState) {
        mPlayerState->setCurrentTrackIndex(-1);
    }
}

size_t AppController::getPlaylistSize() const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    return mPlaylist.size();
}

size_t AppController::loadDirectory(const std::string& directoryPath) {
    // Use filesystem to scan directory for audio files
    size_t count = 0;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                // Convert to lowercase for comparison
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                
                // Check for audio file extensions
                if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac") {
                    addToPlaylist(entry.path().string());
                    count++;
                }
            }
        }
    } catch (const std::exception& e) {
        // Directory not found or permission denied
        (void)e;
    }
    
    return count;
}

std::string AppController::getTrackName(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (index < mPlaylist.size()) {
        return mPlaylist[index].getFilename();
    }
    return "";
}

std::string AppController::getTrackPath(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (index < mPlaylist.size()) {
        return mPlaylist[index].getPath();
    }
    return "";
}

// ============================================================================
// Callbacks
// ============================================================================

void AppController::setStateCallback(AppStateCallback callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mStateCallback = callback;
}

} // namespace Controller
