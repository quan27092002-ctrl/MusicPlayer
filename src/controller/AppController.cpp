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
    // Initialize iterator to end
    mCurrentTrackIterator = mPlaylist.end();
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
    // Expected formats: PLAY, PAUSE, STOP, NEXT, PREV, VOL:50, MUTE, TRACK:5
    
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
    } else if (cmd.rfind("TRACK:", 0) == 0) {
        // Track jump command: TRACK:5
        try {
            int index = std::stoi(cmd.substr(6));
            playTrack(index);
        } catch (...) {
            // Invalid track format
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
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    return getCurrentTrackIndexLocked();
}

int AppController::getCurrentTrackIndexLocked() const {
    if (mPlaylist.empty() || mCurrentTrackIterator == mPlaylist.end()) {
        return -1;
    }

    int index = 0;
    for (auto it = mPlaylist.begin(); it != mPlaylist.end(); ++it) {
        if (it == mCurrentTrackIterator) {
            return index;
        }
        index++;
    }
    return -1;
}

typename std::list<AppController::MediaFilePtr>::iterator AppController::getTrackIterator(int index) {
    if (index < 0 || index >= static_cast<int>(mPlaylist.size())) {
        return mPlaylist.end();
    }
    auto it = mPlaylist.begin();
    std::advance(it, index);
    return it;
}

typename std::list<AppController::MediaFilePtr>::const_iterator AppController::getTrackIteratorConst(int index) const {
    if (index < 0 || index >= static_cast<int>(mPlaylist.size())) {
        return mPlaylist.end();
    }
    auto it = mPlaylist.begin();
    std::advance(it, index);
    return it;
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
    
    // Clear callbacks before final state change
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mStateCallback = nullptr;
    }
    
    // Clear Library and History
    {
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        mHistoryStack.clear();
        mMusicLibrary.clear();
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
        // Find track in playlist and update iterator
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        mCurrentTrackIterator = mPlaylist.end(); // Default to end
        
        int index = 0;
        for (auto it = mPlaylist.begin(); it != mPlaylist.end(); ++it) {
            if ((*it)->getPath() == filePath) {
                mCurrentTrackIterator = it;
                if (mPlayerState) {
                    mPlayerState->setCurrentTrackIndex(index);
                }
                break;
            }
            index++;
        }
    }

    return success;
}

void AppController::play() {
    // ... (play method unchanged - handled by unique_lock fix previously) ...
     if (mAudioPlayer) {
        // Check if we have a valid iterator to play
        bool hasTrack = false;
        {
            std::unique_lock<std::mutex> lock(mPlaylistMutex);
            if (mCurrentTrackIterator != mPlaylist.end()) {
                // If not loaded, load it now
                if (!mAudioPlayer->isLoaded()) {
                    // Need to unlock before loading to avoid deadlock potential 
                     std::string path = (*mCurrentTrackIterator)->getPath();
                     lock.unlock(); // UNLOCK
                     if (loadTrack(path)) {
                        hasTrack = true;
                     }
                } else {
                    hasTrack = true;
                }
            } else if (!mPlaylist.empty()) {
                // Check if we are at start
                mCurrentTrackIterator = mPlaylist.begin();
                std::string path = (*mCurrentTrackIterator)->getPath();
                lock.unlock(); // UNLOCK
                if (loadTrack(path)) {
                    hasTrack = true;
                }
            }
        }

        // If muted, unmute first
        // ...
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

        int currentIndex = getCurrentTrackIndexLocked();
        
        // Push current to history before moving
        if (currentIndex >= 0) {
             // We can't call pushHistory here because we already hold the lock!
             // Direct manipulation or use recursive mutex. 
             // Using direct manipulation for now to avoid modifying helper signature
             if (mHistoryStack.empty() || mHistoryStack.back() != (*mCurrentTrackIterator)) {
                 mHistoryStack.push_back(*mCurrentTrackIterator);
                 if (mHistoryStack.size() > 50) mHistoryStack.erase(mHistoryStack.begin());
             }
        }

        // Move Iterator
        if (mCurrentTrackIterator == mPlaylist.end()) {
            mCurrentTrackIterator = mPlaylist.begin();
        } else {
            mCurrentTrackIterator++;
            if (mCurrentTrackIterator == mPlaylist.end()) {
                mCurrentTrackIterator = mPlaylist.begin(); // Wrap around
            }
        }
        
        if (mCurrentTrackIterator != mPlaylist.end()) {
            pathToLoad = (*mCurrentTrackIterator)->getPath();
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

        // Check history first
        MediaFilePtr historyTrack = popHistory();
        if (historyTrack) {
            pathToLoad = historyTrack->getPath();
            
            // Try to sync iterator if it is in playlist
            mCurrentTrackIterator = mPlaylist.end();
            for (auto it = mPlaylist.begin(); it != mPlaylist.end(); ++it) {
                if ((*it)->getPath() == pathToLoad) {
                    mCurrentTrackIterator = it;
                    break;
                }
            }
        } else {
            // Standard Previous logic
             if (mCurrentTrackIterator == mPlaylist.begin() || mCurrentTrackIterator == mPlaylist.end()) {
                mCurrentTrackIterator = mPlaylist.end();
                mCurrentTrackIterator--; // wrap to last
            } else {
                mCurrentTrackIterator--;
            }
            
            if (mCurrentTrackIterator != mPlaylist.end()) {
                pathToLoad = (*mCurrentTrackIterator)->getPath();
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
         auto it = getTrackIterator(index);
         
         if (it != mPlaylist.end()) {
             // Push old track to history
             if (mCurrentTrackIterator != mPlaylist.end() && mCurrentTrackIterator != it) {
                 if (mHistoryStack.empty() || mHistoryStack.back() != (*mCurrentTrackIterator)) {
                     mHistoryStack.push_back(*mCurrentTrackIterator);
                 }
             }
             
             mCurrentTrackIterator = it;
             pathToLoad = (*it)->getPath();
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
    // 1. Check if song exists in Library
    MediaFilePtr trackPtr = nullptr;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        for (const auto& song : mMusicLibrary) {
            if (song->getPath() == filePath) {
                trackPtr = song;
                break;
            }
        }
    }
    
    // 2. If not in Library, create new and add to Library first
    if (!trackPtr) {
        // Extract metadata first
        std::string filename = filePath;
        size_t lastSlash = filePath.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            filename = filePath.substr(lastSlash + 1);
        }

        std::string artist = "Unknown Artist";
        std::string album = "Unknown Album";
        uint32_t duration = 180;
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

        trackPtr = std::make_shared<Model::MediaFile>(filename, filePath, duration, artist, album, coverArt);
        
        {
            std::lock_guard<std::mutex> lock(mPlaylistMutex);
            mMusicLibrary.push_back(trackPtr);
        }
    }

    // 3. Add pointer to Playlist
    {
        std::lock_guard<std::mutex> lock(mPlaylistMutex);
        mPlaylist.push_back(trackPtr);
        
        // If first track, set iterator
        if (mPlaylist.size() == 1) {
            mCurrentTrackIterator = mPlaylist.begin();
            if (mPlayerState) mPlayerState->setCurrentTrackIndex(0);
        }
    }
}

void AppController::clearPlaylist() {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    mPlaylist.clear();
    mCurrentTrackIterator = mPlaylist.end();
    
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
    auto it = getTrackIteratorConst(static_cast<int>(index));
    if (it != mPlaylist.end()) {
        return (*it)->getFilename();
    }
    return "";
}

std::string AppController::getTrackPath(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    auto it = getTrackIteratorConst(static_cast<int>(index));
    if (it != mPlaylist.end()) {
        return (*it)->getPath();
    }
    return "";
}

std::string AppController::getTrackArtist(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    auto it = getTrackIteratorConst(static_cast<int>(index));
    if (it != mPlaylist.end()) {
        return (*it)->getArtist();
    }
    return "Unknown Artist";
}

std::string AppController::getTrackAlbum(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    auto it = getTrackIteratorConst(static_cast<int>(index));
    if (it != mPlaylist.end()) {
        return (*it)->getAlbum();
    }
    return "Unknown Album";
}

uint32_t AppController::getTrackDuration(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    auto it = getTrackIteratorConst(static_cast<int>(index));
    if (it != mPlaylist.end()) {
        return (*it)->getDuration();
    }
    return 0;
}

std::vector<uint8_t> AppController::getTrackCoverArt(size_t index) const {
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    auto it = getTrackIteratorConst(static_cast<int>(index));
    if (it != mPlaylist.end()) {
        return (*it)->getCoverArt();
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
// History Navigation (Refactored)
// ============================================================================

void AppController::pushHistory(MediaFilePtr track) {
    if (!track) return;
    
    // Safety check: Don't push duplicates if it's the same as top of stack
    if (!mHistoryStack.empty() && mHistoryStack.back() == track) {
        return;
    }
    
    mHistoryStack.push_back(track);
    if (mHistoryStack.size() > 50) {
        mHistoryStack.erase(mHistoryStack.begin());
    }
}

std::shared_ptr<Model::MediaFile> AppController::popHistory() {
    if (mHistoryStack.empty()) {
        return nullptr;
    }
    
    MediaFilePtr track = mHistoryStack.back();
    mHistoryStack.pop_back();
    return track;
}

std::vector<int> AppController::getHistory() const {
    // Legacy support: We have vector<MediaFilePtr>, but interface asks for vector<int>
    // We must map pointers back to current indices in playlist.
    std::lock_guard<std::mutex> lock(mPlaylistMutex);
    std::vector<int> result;
    
    for (const auto& historyItem : mHistoryStack) {
         // Find this item in current playlist
         int idx = 0;
         bool found = false;
         for (auto it = mPlaylist.begin(); it != mPlaylist.end(); ++it) {
             if (*it == historyItem) {
                 result.push_back(idx);
                 found = true;
                 break;
             }
             idx++;
         }
         // If not found (song removed from playlist), we skip it in legacy index view
    }
    return result;
}

} // namespace Controller
