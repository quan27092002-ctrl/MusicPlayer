/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/HistoryManager.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of HistoryManagerImpl.
 */

#include "HistoryManager.h"

namespace Controller {

HistoryManagerImpl::HistoryManagerImpl()
    : mPlaylistRef(nullptr)
    , mPlaylistMutex(nullptr)
{}

std::vector<int> HistoryManagerImpl::getHistory() const {
    std::lock_guard<std::mutex> lock(mMutex);
    std::vector<int> result;
    
    if (!mPlaylistRef || !mPlaylistMutex) {
        return result;
    }
    
    std::lock_guard<std::mutex> playlistLock(*mPlaylistMutex);
    
    for (const auto& historyItem : mHistoryStack) {
        int idx = 0;
        for (auto it = mPlaylistRef->begin(); it != mPlaylistRef->end(); ++it) {
            if (*it == historyItem) {
                result.push_back(idx);
                break;
            }
            idx++;
        }
    }
    return result;
}

size_t HistoryManagerImpl::getHistorySize() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mHistoryStack.size();
}

std::shared_ptr<Model::MediaFile> HistoryManagerImpl::getHistoryItem(size_t index) const {
    std::lock_guard<std::mutex> lock(mMutex);
    if (index >= mHistoryStack.size()) return nullptr;
    return mHistoryStack[index];
}

void HistoryManagerImpl::pushHistory(MediaFilePtr track) {
    if (!track) return;
    
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (!mHistoryStack.empty() && mHistoryStack.back() == track) {
        return;
    }
    
    mHistoryStack.push_back(track);
    if (mHistoryStack.size() > MAX_HISTORY_SIZE) {
        mHistoryStack.erase(mHistoryStack.begin());
    }
}

HistoryManagerImpl::MediaFilePtr HistoryManagerImpl::popHistory() {
    std::lock_guard<std::mutex> lock(mMutex);
    
    if (mHistoryStack.empty()) {
        return nullptr;
    }
    
    MediaFilePtr track = mHistoryStack.back();
    mHistoryStack.pop_back();
    return track;
}

void HistoryManagerImpl::clearHistory() {
    std::lock_guard<std::mutex> lock(mMutex);
    mHistoryStack.clear();
}

void HistoryManagerImpl::setPlaylistRef(const std::list<MediaFilePtr>* playlist, std::mutex* mutex) {
    mPlaylistRef = playlist;
    mPlaylistMutex = mutex;
}

} // namespace Controller
