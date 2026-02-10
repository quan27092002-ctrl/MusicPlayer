/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/PlaybackStateImpl.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of PlaybackStateImpl class.
 */

#include "PlaybackStateImpl.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

PlaybackStateImpl::PlaybackStateImpl()
    : mStatus(PlaybackStatus::STOPPED) {
}

PlaybackStateImpl::PlaybackStateImpl(PlaybackStatus status)
    : mStatus(status) {
}

// ============================================================================
// IPlaybackState Interface Implementation
// ============================================================================

PlaybackStatus PlaybackStateImpl::getPlaybackStatus() const {
    return mStatus.load();
}

void PlaybackStateImpl::setPlaybackStatus(PlaybackStatus status) {
    mStatus.store(status);
}

bool PlaybackStateImpl::isPlaying() const {
    return mStatus.load() == PlaybackStatus::PLAYING;
}

PlaybackStatus PlaybackStateImpl::togglePlayPause() {
    PlaybackStatus current = mStatus.load();
    PlaybackStatus newStatus;

    if (current == PlaybackStatus::PLAYING) {
        newStatus = PlaybackStatus::PAUSED;
    } else {
        newStatus = PlaybackStatus::PLAYING;
    }

    mStatus.store(newStatus);
    return newStatus;
}

// ============================================================================
// Additional Methods
// ============================================================================

void PlaybackStateImpl::reset() {
    mStatus.store(PlaybackStatus::STOPPED);
}

} // namespace Model
