/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/AppController.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of the main application controller.
 *              Facade delegating to composed components following SOLID.
 */

#include "AppController.h"
#include <algorithm>
#include <sstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

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
{
    // Create composed components
    mPlaylistManager = std::make_unique<PlaylistManagerImpl>();
    mVolumeController = std::make_unique<VolumeControllerImpl>(audioPlayer, playerState);
    mHistoryManager = std::make_unique<HistoryManagerImpl>();
    mBoardCommunicator = std::make_unique<BoardCommunicatorImpl>(serialManager, playerState);
    mStorageManager = std::make_unique<StorageManager>(); // Init StorageManager
    mPlaybackController = std::make_unique<PlaybackControllerImpl>(
        audioPlayer, playerState, mPlaylistManager.get(), mHistoryManager.get()
    );
    
    // Setup history manager playlist reference
    mHistoryManager->setPlaylistRef(
        &mPlaylistManager->getPlaylistRef(),
        &mPlaylistManager->getMutex()
    );
    
    // Setup board communicator track index getter
    mBoardCommunicator->setCurrentTrackIndexGetter([this]() {
        return mPlaybackController->getCurrentTrackIndex();
    });
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

// Redefinition removed - see bottom of file
// void AppController::onSerialDataReceived(const std::string& data) {
//    processCommand(data);
// }

void AppController::onSerialStateChanged(SerialState state) {
    if (state == SerialState::CONNECTED) {
        notifyStateChange(AppState::RUNNING);
        mBoardCommunicator->sendStatusToBoard();
    } else if (state == SerialState::DISCONNECTED || state == SerialState::ERROR) {
        if (mAppState.load() == AppState::RUNNING) {
            notifyStateChange(AppState::READY);
        }
    }
}

void AppController::onAudioStateChanged(AudioState state, uint32_t position) {
    (void)position;

    if (mPlayerState) {
        switch (state) {
            case AudioState::PLAYING:
                mPlayerState->setPlaybackState(Model::PlaybackStatus::PLAYING);
                break;
            case AudioState::PAUSED:
                mPlayerState->setPlaybackState(Model::PlaybackStatus::PAUSED);
                break;
            case AudioState::IDLE:
            case AudioState::LOADED:
            case AudioState::ERROR:
                mPlayerState->setPlaybackState(Model::PlaybackStatus::STOPPED);
                break;
            case AudioState::FINISHED:
                mPlayerState->setPlaybackState(Model::PlaybackStatus::STOPPED);
                next(); // Auto-advance to next track
                break;
        }
    }

    mBoardCommunicator->sendStatusToBoard();
}

// ============================================================================
// Board Event Handling
// ============================================================================

void AppController::onBoardEventReceived(BoardEvent event, int value) {
    switch (event) {
        case BoardEvent::PLAY:
            play();
            break;
        case BoardEvent::PAUSE:
            pause();
            break;
        case BoardEvent::STOP:
            stop();
            break;
        case BoardEvent::NEXT:
            next();
            break;
        case BoardEvent::PREV:
            previous();
            break;
        case BoardEvent::SET_VOLUME:
            setVolume(value);
            break;
        default:
            break;
    }
}

void AppController::onSerialDataReceived(const std::string& data) {
    // Forward raw data to BoardCommunicator for parsing (SRP)
    // BoardCommunicator will callback onBoardEventReceived with structured events
    mBoardCommunicator->processCommand(data);
}

// ============================================================================
// IAppLifecycle Implementation
// ============================================================================

bool AppController::initialize() {
    if (mAppState.load() != AppState::UNINITIALIZED) {
        return true;
    }

    if (mAudioPlayer) {
        if (!mAudioPlayer->initialize()) {
            notifyStateChange(AppState::ERROR);
            return false;
        }

        mAudioPlayer->setCallback([this](AudioState state, uint32_t pos) {
            onAudioStateChanged(state, pos);
        });
    }

    if (mSerialManager) {
        mSerialManager->setDataCallback([this](const std::string& data) {
            onSerialDataReceived(data);
        });
        mSerialManager->setStateCallback([this](SerialState state) {
            onSerialStateChanged(state);
        });
    }

    // Setup BoardCommunicator callback
    mBoardCommunicator->setBoardEventCallback([this](BoardEvent event, int value) {
        onBoardEventReceived(event, value);
    });

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

    mPlaylistManager->clearPlaylist();
    mHistoryManager->clearHistory();
    
    {
        std::lock_guard<std::mutex> lock(mCallbackMutex);
        mStateCallback = nullptr;
    }

    mAppState.store(AppState::UNINITIALIZED);
}

AppState AppController::getState() const {
    return mAppState.load();
}

void AppController::setStateCallback(AppStateCallback callback) {
    std::lock_guard<std::mutex> lock(mCallbackMutex);
    mStateCallback = callback;
}

// ============================================================================
// IBoardCommunicator Delegation
// ============================================================================

bool AppController::connectToBoard(const std::string& portName, uint32_t baudRate) {
    return mBoardCommunicator->connectToBoard(portName, baudRate);
}

void AppController::disconnectFromBoard() {
    mBoardCommunicator->disconnectFromBoard();
}

bool AppController::isConnectedToBoard() const {
    return mBoardCommunicator->isConnectedToBoard();
}

std::vector<std::string> AppController::getAvailablePorts() const {
    return mBoardCommunicator->getAvailablePorts();
}

// ============================================================================
// IPlaybackController Delegation
// ============================================================================

bool AppController::loadTrack(const std::string& filePath) {
    return mPlaybackController->loadTrack(filePath);
}

void AppController::play() {
    mPlaybackController->play();
}

void AppController::pause() {
    mPlaybackController->pause();
}

void AppController::stop() {
    mPlaybackController->stop();
}

void AppController::next() {
    mPlaybackController->next();
}

void AppController::previous() {
    mPlaybackController->previous();
}

void AppController::playTrack(int index) {
    mPlaybackController->playTrack(index);
}

void AppController::playPlaylist(const std::vector<std::string>& filePaths) {
    mPlaybackController->replaceQueue(filePaths);
}

void AppController::playLibrary(int startIndex) {
    mPlaybackController->playLibrary(startIndex);
}

void AppController::replaceQueue(const std::vector<std::string>& filePaths) {
    mPlaybackController->replaceQueue(filePaths);
}

void AppController::queuePlaylist(const std::vector<std::string>& filePaths) {
    mPlaybackController->queuePlaylist(filePaths);
}

void AppController::queueNext(const std::string& filePath) {
    mPlaybackController->queueNext(filePath);
}

void AppController::seek(uint32_t positionMs) {
    mPlaybackController->seek(positionMs);
}

// ============================================================================
// IVolumeController Delegation
// ============================================================================

void AppController::setVolume(int volume) {
    mVolumeController->setVolume(volume);
    mBoardCommunicator->sendStatusToBoard();
}

int AppController::getVolume() const {
    return mVolumeController->getVolume();
}

void AppController::toggleMute() {
    mVolumeController->toggleMute();
    mBoardCommunicator->sendStatusToBoard();
}

// ============================================================================
// IPlaylistManager Delegation
// ============================================================================

void AppController::addToPlaylist(const std::string& filePath) {
    mPlaylistManager->addToPlaylist(filePath);
    
    // Update playback controller iterator if first track
    if (mPlaylistManager->getPlaylistSize() == 1) {
        auto& playlist = mPlaylistManager->getPlaylistRef();
        mPlaybackController->getCurrentIterator() = playlist.begin();
        if (mPlayerState) {
            mPlayerState->setCurrentTrackIndex(0);
        }
    }
}

void AppController::clearPlaylist() {
    mPlaylistManager->clearPlaylist();
    mPlaybackController->getCurrentIterator() = mPlaylistManager->getPlaylistRef().end();
    if (mPlayerState) {
        mPlayerState->setCurrentTrackIndex(-1);
    }
}

size_t AppController::getPlaylistSize() const {
    return mPlaylistManager->getPlaylistSize();
}

size_t AppController::loadDirectory(const std::string& directoryPath) {
    size_t count = mPlaylistManager->loadDirectory(directoryPath);
    
    // Update playback controller iterator if tracks were added
    if (count > 0 && mPlaybackController->getCurrentTrackIndex() < 0) {
        auto& playlist = mPlaylistManager->getPlaylistRef();
        if (!playlist.empty()) {
            mPlaybackController->getCurrentIterator() = playlist.begin();
            if (mPlayerState) {
                mPlayerState->setCurrentTrackIndex(0);
            }
        }
    }
    
    return count;
}

std::string AppController::getTrackName(size_t index) const {
    return mPlaylistManager->getTrackName(index);
}

std::string AppController::getTrackPath(size_t index) const {
    return mPlaylistManager->getTrackPath(index);
}

std::string AppController::getTrackArtist(size_t index) const {
    return mPlaylistManager->getTrackArtist(index);
}

std::string AppController::getTrackAlbum(size_t index) const {
    return mPlaylistManager->getTrackAlbum(index);
}

uint32_t AppController::getTrackDuration(size_t index) const {
    return mPlaylistManager->getTrackDuration(index);
}

std::vector<uint8_t> AppController::getTrackCoverArt(size_t index) const {
    return mPlaylistManager->getTrackCoverArt(index);
}

std::shared_ptr<Model::MediaFile> AppController::acquireMediaFile(const std::string& filePath) {
    return mPlaylistManager->acquireMediaFile(filePath);
}

// Library Accessors Implementation
size_t AppController::getLibrarySize() const {
    return mPlaylistManager->getLibrarySize();
}
std::string AppController::getLibraryTrackName(size_t index) const {
    return mPlaylistManager->getLibraryTrackName(index);
}
std::string AppController::getLibraryTrackPath(size_t index) const {
    return mPlaylistManager->getLibraryTrackPath(index);
}
std::string AppController::getLibraryTrackArtist(size_t index) const {
    return mPlaylistManager->getLibraryTrackArtist(index);
}
std::string AppController::getLibraryTrackAlbum(size_t index) const {
    return mPlaylistManager->getLibraryTrackAlbum(index);
}
std::vector<uint8_t> AppController::getLibraryTrackCoverArt(size_t index) const {
    return mPlaylistManager->getLibraryTrackCoverArt(index);
}

// ============================================================================
// IHistoryManager Delegation
// ============================================================================
// IHistoryManager Delegation
// ============================================================================

std::vector<int> AppController::getHistory() const {
    return mHistoryManager->getHistory();
}

std::shared_ptr<Model::MediaFile> AppController::getHistoryItem(size_t index) const {
    return mHistoryManager->getHistoryItem(index);
}

size_t AppController::getHistorySize() const {
    return mHistoryManager->getHistorySize();
}

std::string AppController::getHistoryTrackName(size_t index) const {
    auto item = mHistoryManager->getHistoryItem(index);
    return item ? item->getFilename() : "Unknown";
}

std::string AppController::getHistoryTrackArtist(size_t index) const {
    auto item = mHistoryManager->getHistoryItem(index);
    return item ? item->getArtist() : "Unknown";
}

std::string AppController::getHistoryTrackAlbum(size_t index) const {
    auto item = mHistoryManager->getHistoryItem(index);
    return item ? item->getAlbum() : "Unknown";
}

std::vector<uint8_t> AppController::getHistoryTrackCoverArt(size_t index) const {
    auto item = mHistoryManager->getHistoryItem(index);
    return item ? item->getCoverArt() : std::vector<uint8_t>{};
}

void AppController::playHistoryTrack(size_t index) {
    auto item = mHistoryManager->getHistoryItem(index);
    if (item && !item->getPath().empty()) {
        playPlaylist({item->getPath()});
    }
}

void AppController::setPlaylistUpdatedCallback(std::function<void()> callback) {
    // Forward to PlaylistManager
    mPlaylistManager->setPlaylistUpdatedCallback(callback); 
    // Wait, PlaylistManager callback calls THIS callback?
    // PlaylistManager stores a std::function. 
    // Yes, we just pass the View's callback into PlaylistManager.
    // Or we could wrap it if we needed to do AppController logic.
    // For now, direct forwarding is fine.
}

void AppController::notifyPlaylistUpdated() {
    mPlaylistManager->notifyPlaylistUpdated();
}

// ============================================================================
// Storage Management
// ============================================================================

std::vector<StorageDevice> AppController::getStorageDevices() {
    if (mStorageManager) {
        return mStorageManager->getAvailableStorage();
    }
    return {};
}

size_t AppController::loadFromStorage(const std::string& path) {
    // Forward to PlaylistManager via loadDirectory
    // User wants to APPEND to existing library (mMusic + USBs)
    // So we do NOT clearPlaylist().
    
    // Potential improvement: Check if this path was already loaded?
    // For now, relies on PlaylistManager to handle or just append.
    // If PlaylistManager::loadDirectory just scans and calls acquireMediaFile,
    // and if acquireMediaFile is idempotent for library but NOT for playlist...
    
    // Wait, PlaylistManager::loadDirectory currently calls acquireMediaFile but DOES NOT add to playlist queue automatically?
    // Let's check PlaylistManager::loadDirectory implementation.
    // It calls acquireMediaFile. acquireMediaFile adds to mMusicLibrary if new.
    // It DOES NOT add to mPlaylist.
    
    // Wait, if it doesn't add to playlist, how does music play?
    // Ah, `AppController::loadDirectory` logic:
    /*
    size_t count = mPlaylistManager->loadDirectory(directoryPath);
    // Update playback controller iterator if tracks were added
    */
   
    // Actually, `PlaylistManager::loadDirectory` returns count of found files.
    // But it does NOT add them to `mPlaylist` (the playback queue).
    // It only adds to `mMusicLibrary`.
    // We need to add them to `mPlaylist` if we want them playable in the queue.
    
    // Let's verify `PlaylistManager::loadDirectory`.
    // It iterates and calls `acquireMediaFile`. 
    // `acquireMediaFile` adds to `mMusicLibrary` if not exists.
    // It returns the pointer.
    // BUT `loadDirectory` discards the pointer!
    
    // So `loadDirectory` ONLY populates the Library, not the Queue.
    // If we want to play them, we need to add them to the queue.
    
    // Implementation in AppController::loadDirectory:
    // It calls mPlaylistManager->loadDirectory.
    // Then checks mPlaybackController->getCurrentTrackIndex().

    // NOTE: The separate `AppController::loadDirectory` implementation:
    /*
    size_t AppController::loadDirectory(const std::string& directoryPath) {
        size_t count = mPlaylistManager->loadDirectory(directoryPath);
        ...
    }
    */

    // Conclusion: The current `loadDirectory` implementation in AppController matches `mPlaylistManager->loadDirectory`.
    // NEITHER adds to the queue.
    // This explains why the user might see "Loaded" but not see them in the queue?
    // Wait, `mMusic` loading at startup works. Why?
    // Maybe `MainContent` or `PlayerBar` displays the *Library*?
    // `RightSidebar` displays `mPlaylist`.
    
    // I need to change `AppController::loadDirectory` (or create a new method) to ALSO add found files to the Playlist.
    // OR change `PlaylistManager::loadDirectory` to add to playlist.
    
    // Let's modify `AppController::loadFromStorage` to:
    // 1. Call `loadDirectory` (populates library)
    // 2. Scan directory again to add to Playlist? Or make `PlaylistManager` return the added files?
    
    // Ideally, `loadDirectory` should optionally add to playlist.
    // Or I iterate the library and add everything?
    
    // Let's modify `PlaylistManager` to have `loadDirectoryToPlaylist`?
    // Or just `loadFromStorage` calls `addToPlaylist` for every file found?
    
    // Better approach:
    // `mPlaylistManager->loadDirectory` returns count. 
    // But we need the files.
    
    // Let's change `loadFromStorage` to iterate the directory and call `addToPlaylist`.
    // `addToPlaylist` adds to both Library (via acquire) and Queue.

    size_t count = 0;
    namespace fs = std::filesystem;
    if (fs::exists(path) && fs::is_directory(path)) {
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac") {
                    // Only add to Library, not Queue
                    acquireMediaFile(entry.path().string());
                    count++;
                }
            }
        }
    }
    
    // Do not notify playlist update as queue hasn't changed
    // if (count > 0) {
    //     notifyPlaylistUpdated();
    // }

    return count;
}

} // namespace Controller
