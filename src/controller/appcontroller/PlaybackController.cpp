/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/PlaybackController.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of PlaybackControllerImpl.
 */

#include "PlaybackController.h"

namespace Controller {

PlaybackControllerImpl::PlaybackControllerImpl(
    std::shared_ptr<IAudioPlayer> audioPlayer,
    std::shared_ptr<Model::IPlayerState> playerState,
    PlaylistManagerImpl* playlistManager,
    HistoryManagerImpl* historyManager
)
    : mAudioPlayer(audioPlayer)
    , mPlayerState(playerState)
    , mPlaylistManager(playlistManager)
    , mHistoryManager(historyManager)
    , mStatusCallback(nullptr)
{
    // Initialize iterator
    if (mPlaylistManager) {
        mCurrentTrackIterator = mPlaylistManager->getPlaylistRef().end();
    }
}

bool PlaybackControllerImpl::loadTrack(const std::string& filePath) {
    if (!mAudioPlayer || !mPlaylistManager) {
        return false;
    }

    bool success = mAudioPlayer->load(filePath);
    
    if (success) {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        mCurrentTrackIterator = playlist.end();
        
        int index = 0;
        for (auto it = playlist.begin(); it != playlist.end(); ++it) {
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

void PlaybackControllerImpl::play() {
    if (!mAudioPlayer || !mPlaylistManager) return;
    
    {
        std::unique_lock<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (mCurrentTrackIterator != playlist.end()) {
            if (!mAudioPlayer->isLoaded()) {
                std::string path = (*mCurrentTrackIterator)->getPath();
                lock.unlock();
                loadTrack(path);
            }
        } else if (!playlist.empty()) {
            mCurrentTrackIterator = playlist.begin();
            std::string path = (*mCurrentTrackIterator)->getPath();
            lock.unlock();
            loadTrack(path);
        }
    }

    if (mPlayerState && mPlayerState->isMuted()) {
        mPlayerState->setMuted(false);
        mAudioPlayer->setVolume(mPlayerState->getVolume());
    }
    
    mAudioPlayer->play();
}

void PlaybackControllerImpl::pause() {
    if (mAudioPlayer) {
        mAudioPlayer->pause();
    }
}

void PlaybackControllerImpl::stop() {
    if (mAudioPlayer) {
        mAudioPlayer->stop();
    }
}

void PlaybackControllerImpl::next() {
    if (!mPlaylistManager) return;
    
    std::string pathToLoad;
    bool shouldNotify = false;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (playlist.empty()) return;

    // Consumer Mode:
        // 1. Push current to history (if valid)
        // 2. Erase current from queue (UNLESS it's the last one)
        // 3. Play next

        // Safety: ensure iterator is valid
        if (mCurrentTrackIterator != playlist.end()) {
            
            // Check if this is the last track
            auto nextIt = mCurrentTrackIterator;
            std::advance(nextIt, 1);
            bool isLastTrack = (nextIt == playlist.end());

            if (isLastTrack) {
                // It is the last track. Use Special Behavior.
                // 1. Push to history (so it shows in history too)
                if (mHistoryManager) {
                    mHistoryManager->pushHistory(*mCurrentTrackIterator);
                }
                
                // 2. DO NOT erase. Keep it as "Current".
                // We want the UI to show this track as "Current Song" even if stopped.
                
                // 3. Do NOT set pathToLoad, so we enter the "Queue finished" block below.
                shouldNotify = true; // Helper to ensure history update is reflected if needed
                
                // Stop playback state manually here or let the "else" block handle it?
                // The "else" block handles it if pathToLoad is empty.
                
            } else {
                // NOT the last track. Normal Consumer Behavior.
                if (mHistoryManager) {
                    mHistoryManager->pushHistory(*mCurrentTrackIterator);
                }
                
                // Advance iterator by erasing current
                // erase returns iterator following the removed element
                mCurrentTrackIterator = playlist.erase(mCurrentTrackIterator);
                
                shouldNotify = true;
                
                 // If we still have tracks (which we should, since it wasn't last)
                if (mCurrentTrackIterator != playlist.end()) {
                    pathToLoad = (*mCurrentTrackIterator)->getPath();
                }
            }
        } else {
             // If iterator was already at end (shouldn't happen if playing, but maybe manual next at end?)
             if (!playlist.empty()) {
                  mCurrentTrackIterator = playlist.begin();
                  pathToLoad = (*mCurrentTrackIterator)->getPath();
             }
        }
    }

    // Call notification OUTSIDE the lock
    if (shouldNotify) {
        mPlaylistManager->notifyPlaylistUpdated();
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    } else {
        // Queue finished
        // Ensure state is stopped
        if (mPlayerState) {
            mPlayerState->setPlaybackStatus(Model::PlaybackStatus::STOPPED);
        }
    }
}

void PlaybackControllerImpl::previous() {
    if (!mPlaylistManager) return;
    
    std::string pathToLoad;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (playlist.empty()) return;

        // Check history first
        MediaFilePtr historyTrack = mHistoryManager ? mHistoryManager->popHistory() : nullptr;
        if (historyTrack) {
            pathToLoad = historyTrack->getPath();
            
            mCurrentTrackIterator = playlist.end();
            for (auto it = playlist.begin(); it != playlist.end(); ++it) {
                if ((*it)->getPath() == pathToLoad) {
                    mCurrentTrackIterator = it;
                    break;
                }
            }
        } else {
            if (mCurrentTrackIterator == playlist.begin() || mCurrentTrackIterator == playlist.end()) {
                mCurrentTrackIterator = playlist.end();
                --mCurrentTrackIterator;
            } else {
                --mCurrentTrackIterator;
            }
            
            if (mCurrentTrackIterator != playlist.end()) {
                pathToLoad = (*mCurrentTrackIterator)->getPath();
            }
        }
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    }
}

void PlaybackControllerImpl::playTrack(int index) {
    if (!mPlaylistManager) return;
    
    std::string pathToLoad;
    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto it = getTrackIterator(index);
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (it != playlist.end()) {
            // Push old track to history
            if (mCurrentTrackIterator != playlist.end() && mCurrentTrackIterator != it && mHistoryManager) {
                mHistoryManager->pushHistory(*mCurrentTrackIterator);
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

void PlaybackControllerImpl::seek(uint32_t positionMs) {
    if (mAudioPlayer) {
        mAudioPlayer->seek(positionMs);
    }
}

void PlaybackControllerImpl::queueNext(const std::string& filePath) {
    if (!mPlaylistManager) return;
    
    auto trackPtr = mPlaylistManager->acquireMediaFile(filePath);
    if (!trackPtr) return;
    
    bool shouldNotify = false;

    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (playlist.empty()) {
            playlist.push_back(trackPtr);
            mCurrentTrackIterator = playlist.begin(); // Ready to play
        } else {
            // Insert AFTER current track
            auto insertPos = mCurrentTrackIterator;
            if (insertPos != playlist.end()) {
                insertPos++; 
            } else {
                insertPos = playlist.end();
            }
            playlist.insert(insertPos, trackPtr);
            
            // If nothing was playing (stopped state with non-empty playlist?), 
            // this just adds to queue.
            // If we want to move iterator if it was at end?
            if (mCurrentTrackIterator == playlist.end()) {
                mCurrentTrackIterator = playlist.begin(); // Reset if we were at end
            }
        }
        shouldNotify = true;
    }

    if (shouldNotify) {
        mPlaylistManager->notifyPlaylistUpdated();
    }
}

void PlaybackControllerImpl::replaceQueue(const std::vector<std::string>& filePaths) {
    if (!mPlaylistManager) return;

    // 1. Acquire MediaFiles OUTSIDE the lock
    std::vector<MediaFilePtr> newTracks;
    newTracks.reserve(filePaths.size());
    for (const auto& path : filePaths) {
        auto ptr = mPlaylistManager->acquireMediaFile(path);
        if (ptr) newTracks.push_back(ptr);
    }
    
    bool shouldNotify = false;

    // 2. Lock and Replace
    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        playlist.clear();
        for (const auto& track : newTracks) {
            playlist.push_back(track);
        }
        
        // 3. Reset Iterator
        if (!playlist.empty()) {
            mCurrentTrackIterator = playlist.begin();
        } else {
            mCurrentTrackIterator = playlist.end();
        }
        shouldNotify = true;
    }

    if (shouldNotify) {
        mPlaylistManager->notifyPlaylistUpdated();
    }
    
    // 4. Play
    if (!filePaths.empty()) {
        play();
    } else {
        stop();
    }
}

void PlaybackControllerImpl::queuePlaylist(const std::vector<std::string>& filePaths) {
    if (!mPlaylistManager) return;

    std::vector<MediaFilePtr> newTracks;
    newTracks.reserve(filePaths.size());
    for (const auto& path : filePaths) {
        auto ptr = mPlaylistManager->acquireMediaFile(path);
        if (ptr) newTracks.push_back(ptr);
    }
    
    if (newTracks.empty()) return;

    bool shouldNotify = false;

    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        bool wasEmpty = playlist.empty();
        
        for (const auto& track : newTracks) {
            playlist.push_back(track);
        }
        
        if (wasEmpty) {
            mCurrentTrackIterator = playlist.begin();
        }
        
        shouldNotify = true;
    }

    if (shouldNotify) {
        mPlaylistManager->notifyPlaylistUpdated();
    }
}

int PlaybackControllerImpl::getCurrentTrackIndex() const {
    if (!mPlaylistManager) return -1;
    std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
    return getCurrentTrackIndexLocked();
}

int PlaybackControllerImpl::getCurrentTrackIndexLocked() const {
    if (!mPlaylistManager) return -1;
    const auto& playlist = mPlaylistManager->getPlaylistRef();
    
    if (playlist.empty() || mCurrentTrackIterator == playlist.end()) {
        return -1;
    }

    int index = 0;
    for (auto it = playlist.begin(); it != playlist.end(); ++it) {
        if (it == mCurrentTrackIterator) {
            return index;
        }
        index++;
    }
    return -1;
}

typename std::list<PlaybackControllerImpl::MediaFilePtr>::iterator 
PlaybackControllerImpl::getTrackIterator(int index) {
    auto& playlist = mPlaylistManager->getPlaylistRef();
    if (index < 0 || index >= static_cast<int>(playlist.size())) {
        return playlist.end();
    }
    auto it = playlist.begin();
    std::advance(it, index);
    return it;
}

void PlaybackControllerImpl::setStatusCallback(std::function<void()> callback) {
    mStatusCallback = callback;
}

typename std::list<PlaybackControllerImpl::MediaFilePtr>::iterator& 
PlaybackControllerImpl::getCurrentIterator() {
    return mCurrentTrackIterator;
}


} // namespace Controller
