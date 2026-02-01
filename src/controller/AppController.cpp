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
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <cctype> // for std::tolower

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
            case AudioState::FINISHED:
                mPlayerState->setPlaybackState(Model::PlaybackState::STOPPED);
                next(); // Auto-advance to next track
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
    
    // Clear playlist to free MediaFiles/CoverArt while we are still alive
    {
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        mPlaylist.clear();
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
        
        // Push current to history before moving
        if (currentIndex >= 0) {
             // We can't call pushHistory here because we already hold the lock!
             // Direct manipulation or use recursive mutex. 
             // Using direct manipulation for now to avoid modifying helper signature
             if (mHistoryStack.empty() || mHistoryStack.back() != currentIndex) {
                 mHistoryStack.push_back(currentIndex);
                 if (mHistoryStack.size() > 50) mHistoryStack.erase(mHistoryStack.begin());
             }
        }

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

        // Try to pop from history 
        if (!mHistoryStack.empty()) {
            int historyIndex = mHistoryStack.back();
            mHistoryStack.pop_back();
            
            if (historyIndex >= 0 && historyIndex < static_cast<int>(mPlaylist.size())) {
                pathToLoad = mPlaylist[historyIndex].getPath();
            }
        } else {
            // Fallback: Standard previous
            int currentIndex = getCurrentTrackIndex();
            int prevIndex = currentIndex - 1;
            if (prevIndex < 0) {
                prevIndex = static_cast<int>(mPlaylist.size()) - 1;
            }
            
            if (prevIndex >= 0 && prevIndex < static_cast<int>(mPlaylist.size())) {
                pathToLoad = mPlaylist[prevIndex].getPath();
            }
        }
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    }
}

void AppController::playTrack(int index) {
    std::string pathToLoad;
    {
         std::lock_guard<std::mutex> lock(mPlaylistMutex);
         if (index >= 0 && index < static_cast<int>(mPlaylist.size())) {
             int currentIndex = getCurrentTrackIndex();
             if (currentIndex >= 0 && currentIndex != index) {
                 // Push old track to history
                 if (mHistoryStack.empty() || mHistoryStack.back() != currentIndex) {
                     mHistoryStack.push_back(currentIndex);
                 }
             }
             pathToLoad = mPlaylist[index].getPath();
         }
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    }
}

void AppController::seek(uint32_t positionMs) {
    if (mAudioPlayer) {
        mAudioPlayer->seek(positionMs);
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

    // Read metadata using TagLib
    std::string artist = "Unknown Artist";
    std::string album = "Unknown Album";
    uint32_t duration = 0;
    std::vector<uint8_t> coverArt;
    
    std::string ext;
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        ext = filePath.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
    }

    // Use FileRef for robust file opening
    TagLib::FileRef f(filePath.c_str());
    if (!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        
        // Basic Metadata
        if (!tag->artist().isEmpty()) artist = tag->artist().toCString(true);
        if (!tag->album().isEmpty()) album = tag->album().toCString(true);
        
        // Duration
        if (f.audioProperties()) {
            duration = f.audioProperties()->lengthInSeconds();
        }

        // Try to extract ID3v2 Cover Art (APIC)
        // Verify it's an MPEG file specifically for ID3v2 access
        if (ext == ".mp3") {
            TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(f.file());
            if (mpegFile && mpegFile->ID3v2Tag()) {
                 TagLib::ID3v2::Tag *id3v2 = mpegFile->ID3v2Tag();
                 TagLib::ID3v2::FrameList frames = id3v2->frameListMap()["APIC"];
                 
                 if (!frames.isEmpty()) {
                     TagLib::ID3v2::AttachedPictureFrame *frame = 
                         dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
                     if (frame) {
                         TagLib::ByteVector pic = frame->picture();
                         if (pic.size() > 0 && pic.size() < 5*1024*1024) { // 5MB Limit
                             // Safer copy
                             coverArt.reserve(pic.size());
                             const char* data = pic.data();
                             coverArt.assign(data, data + pic.size());
                         }
                     }
                 }
            }
        }
    }
    
    // Default duration/metadata fallback not needed as initialized above
    if (duration == 0) duration = 180;

    Model::MediaFile file(filename, filePath, duration, artist, album, coverArt);
    
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
                std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c){ return std::tolower(c); });
                
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

std::string AppController::getTrackArtist(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (index < mPlaylist.size()) {
        return mPlaylist[index].getArtist();
    }
    return "Unknown Artist";
}

std::string AppController::getTrackAlbum(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (index < mPlaylist.size()) {
        return mPlaylist[index].getAlbum();
    }
    return "Unknown Album";
}

uint32_t AppController::getTrackDuration(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (index < mPlaylist.size()) {
        return mPlaylist[index].getDuration();
    }
    return 0;
}

std::vector<uint8_t> AppController::getTrackCoverArt(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (index < mPlaylist.size()) {
        return mPlaylist[index].getCoverArt();
    }
    return {};
}

// ============================================================================
// Callbacks
// ============================================================================

void AppController::setStateCallback(AppStateCallback callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mStateCallback = callback;
}

// ============================================================================
// History Navigation
// ============================================================================

void AppController::pushHistory(int trackIndex) {
    if (trackIndex < 0) return;
    
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    
    // Avoid checking duplicates against the *very top* if we want strict history trace
    // But usually we don't want "Song 1 -> Song 1" to add to history.
    if (!mHistoryStack.empty() && mHistoryStack.back() == trackIndex) {
        return;
    }
    
    mHistoryStack.push_back(trackIndex);
    // Limit stack size if needed (e.g., 50)
    if (mHistoryStack.size() > 50) {
        mHistoryStack.erase(mHistoryStack.begin());
    }
}

int AppController::popHistory() {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    if (mHistoryStack.empty()) {
        return -1;
    }
    
    int index = mHistoryStack.back();
    mHistoryStack.pop_back();
    return index;
}

std::vector<int> AppController::getHistory() const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    // Return copy for thread safety
    // User wants UI to show history. Usually "Most recent at top".
    // Vector is [Oldest, ..., Newest].
    // So UI should iterate in reverse. We just return the raw stack.
    return mHistoryStack;
}

} // namespace Controller
