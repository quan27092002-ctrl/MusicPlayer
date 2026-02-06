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

void AppController::onSerialDataReceived(const std::string& data) {
    processCommand(data);
}

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

void AppController::processCommand(const std::string& command) {
    std::string cmd = command;
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
        try {
            int vol = std::stoi(cmd.substr(4));
            setVolume(vol);
        } catch (...) {}
    } else if (cmd.rfind("TRACK:", 0) == 0) {
        try {
            int index = std::stoi(cmd.substr(6));
            playTrack(index);
        } catch (...) {}
    } else if (cmd.rfind("LOAD:", 0) == 0) {
        std::string path = command.substr(5);
        loadTrack(path);
    } else if (cmd == "STATUS") {
        mBoardCommunicator->sendStatusToBoard();
    }
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

std::vector<int> AppController::getHistory() const {
    return mHistoryManager->getHistory();
}

} // namespace Controller
