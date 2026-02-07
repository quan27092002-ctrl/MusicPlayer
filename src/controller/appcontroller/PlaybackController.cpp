/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/PlaybackController.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of PlaybackControllerImpl.
 */

#include "PlaybackController.h"
#include <iostream>

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
        mCurrentLoadedPath = filePath; // Track loaded path
        
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
    } else {
        // Optional: clear if load failed? Or keep old?
        // Usually if load fails, state is undefined / old track might still be there but error occurred.
        // Let's safe keep old or clear? 
        // If load fails, AudioPlayer might be in weird state.
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
    
    // Only play if we have a valid track loaded or selected
    bool canPlay = false;
    std::string pathToString;
    bool needsLoad = false;
    
    {
        // quick check without lock if possible, or reliance on flow
        // Re-locking to check iterator validity is safest
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        if (mCurrentTrackIterator != mPlaylistManager->getPlaylistRef().end()) {
            canPlay = true;
            pathToString = (*mCurrentTrackIterator)->getPath();
        }
    }
    
    if (canPlay) {
        // Strict check: Is the audio player actually holding this file?
        // Even if isLoaded() is true, it might be the OLD file.
        if (mCurrentLoadedPath != pathToString) {
             needsLoad = true;
        }
    }

    if (canPlay) {
        if (needsLoad) {
             loadTrack(pathToString);
             // loadTrack updates mCurrentLoadedPath and usually resets iterator logic, 
             // but path is same so iterator remains valid/correct.
        }
        mAudioPlayer->play();
    }
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
    
    // Check repeat mode - if enabled, replay current track
    if (mPlayerState && mPlayerState->getRepeatMode() == Model::RepeatMode::ONE) {
        if (mAudioPlayer) {
            mAudioPlayer->seek(0);
            mAudioPlayer->play();
        }
        // Signal UI to reset position tracking
        if (mPlayerState) {
            mPlayerState->setCurrentPosition(0);
            mPlayerState->incrementPlaybackVersion();
        }
        return;
    }
    
    std::string pathToLoad;
    bool shouldNotify = false;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (playlist.empty()) return;

    // Consumer Mode:
        // 1. Push current to history (if valid)
        // 2. Erase current from queue
        // 3. Play next

        // Safety: ensure iterator is valid
        if (mCurrentTrackIterator != playlist.end()) {
            
            // Store the path before the track is erased
            std::string consumedPath = (*mCurrentTrackIterator)->getPath();
            
            if (mHistoryManager) {
                mHistoryManager->pushHistory(*mCurrentTrackIterator);
            }
            
            // Also remove from original order if shuffle is enabled
            // so restore doesn't bring back already-played songs
            if (mPlayerState && mPlayerState->isShuffleEnabled() && !mOriginalOrder.empty()) {
                mOriginalOrder.erase(
                    std::remove_if(mOriginalOrder.begin(), mOriginalOrder.end(),
                        [&consumedPath](const MediaFilePtr& track) {
                            return track->getPath() == consumedPath;
                        }),
                    mOriginalOrder.end());
            }
            
            // Advance iterator by erasing current
            // erase returns iterator following the removed element
            mCurrentTrackIterator = playlist.erase(mCurrentTrackIterator);
            
            shouldNotify = true;
            
             // If we still have tracks
            if (mCurrentTrackIterator != playlist.end()) {
                pathToLoad = (*mCurrentTrackIterator)->getPath();
            }
        } else {
             // If iterator was already at end
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
        if (mAudioPlayer) {
            mAudioPlayer->stop();
        }
        // Ensure state is stopped
        if (mPlayerState) {
            mPlayerState->setPlaybackStatus(Model::PlaybackStatus::STOPPED);
            mPlayerState->setCurrentTrackIndex(-1);
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
            mCurrentTrackIterator = playlist.begin();
            // Auto-play if queue was empty
            if (mAudioPlayer) {
                 if (mAudioPlayer->load(trackPtr->getPath())) {
                     mAudioPlayer->play();
                     if (mPlayerState) {
                         mPlayerState->setPlaybackStatus(Model::PlaybackStatus::PLAYING);
                         // CRITICAL: Update current track index for UI
                         mPlayerState->setCurrentTrackIndex(0); 
                     }
                 }
            }
        } else {
            // Insert AFTER current track
            auto insertPos = mCurrentTrackIterator;
            if (insertPos != playlist.end()) {
                insertPos++; 
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

void PlaybackControllerImpl::playLibrary(int startIndex) {
    if (!mPlaylistManager) return;
    
    bool shouldNotify = false;
    std::string pathToLoad;

    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        
        // 1. Copy Library to Playlist
        auto& library = mPlaylistManager->getMusicLibraryRef();
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        playlist.clear();
        for (const auto& track : library) {
            playlist.push_back(track);
        }
        
        // 2. Set Iterator
        if (!playlist.empty()) {
            if (startIndex >= 0 && startIndex < (int)playlist.size()) {
                mCurrentTrackIterator = playlist.begin();
                std::advance(mCurrentTrackIterator, startIndex);
                pathToLoad = (*mCurrentTrackIterator)->getPath();
            } else {
                mCurrentTrackIterator = playlist.begin();
                pathToLoad = (*mCurrentTrackIterator)->getPath();
            }
        } else {
            mCurrentTrackIterator = playlist.end();
        }
        
        shouldNotify = true;
    }
    
    if (shouldNotify) {
        mPlaylistManager->notifyPlaylistUpdated();
    }
    
    if (!pathToLoad.empty()) {
        loadTrack(pathToLoad);
        play();
    } else {
        stop();
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

void PlaybackControllerImpl::toggleShuffle() {
    if (!mPlaylistManager || !mPlayerState) return;
    
    bool wasShuffled = mPlayerState->isShuffleEnabled();
    bool shouldNotify = false;
    
    {
        std::lock_guard<std::mutex> lock(mPlaylistManager->getMutex());
        auto& playlist = mPlaylistManager->getPlaylistRef();
        
        if (wasShuffled) {
            // Restore original order of remaining songs (after current)
            if (!mOriginalOrder.empty() && !playlist.empty()) {
                // Keep the current track (first in queue)
                MediaFilePtr currentTrack = nullptr;
                if (mCurrentTrackIterator != playlist.end()) {
                    currentTrack = *mCurrentTrackIterator;
                }
                
                // Clear everything after current
                if (currentTrack) {
                    playlist.clear();
                    playlist.push_back(currentTrack);
                    
                    // Add remaining songs from original order
                    for (const auto& track : mOriginalOrder) {
                        playlist.push_back(track);
                    }
                    mOriginalOrder.clear();
                    
                    // Current is still at the front
                    mCurrentTrackIterator = playlist.begin();
                }
            }
            mPlayerState->setShuffleEnabled(false);
        } else {
            // Save original order of songs AFTER current (not including current)
            mOriginalOrder.clear();
            
            if (!playlist.empty() && mCurrentTrackIterator != playlist.end()) {
                // Save songs after current in their original order
                auto it = mCurrentTrackIterator;
                ++it; // Skip current track
                for (; it != playlist.end(); ++it) {
                    mOriginalOrder.push_back(*it);
                }
                
                // Shuffle only the songs after current
                if (!mOriginalOrder.empty()) {
                    std::vector<MediaFilePtr> toShuffle(mOriginalOrder.begin(), mOriginalOrder.end());
                    std::random_shuffle(toShuffle.begin(), toShuffle.end());
                    
                    // Rebuild playlist: current track + shuffled remaining
                    MediaFilePtr currentTrack = *mCurrentTrackIterator;
                    playlist.clear();
                    playlist.push_back(currentTrack);
                    for (const auto& track : toShuffle) {
                        playlist.push_back(track);
                    }
                    
                    mCurrentTrackIterator = playlist.begin();
                }
            }
            mPlayerState->setShuffleEnabled(true);
        }
        shouldNotify = true;
    }
    
    if (shouldNotify) {
        mPlaylistManager->notifyPlaylistUpdated();
    }
}

void PlaybackControllerImpl::toggleRepeat() {
    if (!mPlayerState) return;
    
    auto current = mPlayerState->getRepeatMode();
    if (current == Model::RepeatMode::NONE) {
        mPlayerState->setRepeatMode(Model::RepeatMode::ONE);
    } else {
        mPlayerState->setRepeatMode(Model::RepeatMode::NONE);
    }
}


} // namespace Controller

