/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/PlaylistNavigationImpl.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of PlaylistNavigationImpl class.
 */

#include "PlaylistNavigationImpl.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

PlaylistNavigationImpl::PlaylistNavigationImpl()
    : mTrackIndex(-1)
    , mRepeatMode(RepeatMode::NONE)
    , mShuffleEnabled(false) {
}

PlaylistNavigationImpl::PlaylistNavigationImpl(int trackIndex, 
                                               RepeatMode repeatMode, 
                                               bool shuffleEnabled)
    : mTrackIndex(trackIndex)
    , mRepeatMode(repeatMode)
    , mShuffleEnabled(shuffleEnabled) {
}

// ============================================================================
// IPlaylistNavigation Interface Implementation
// ============================================================================

int PlaylistNavigationImpl::getCurrentTrackIndex() const {
    return mTrackIndex.load();
}

void PlaylistNavigationImpl::setCurrentTrackIndex(int index) {
    mTrackIndex.store(index);
}

RepeatMode PlaylistNavigationImpl::getRepeatMode() const {
    return mRepeatMode.load();
}

void PlaylistNavigationImpl::setRepeatMode(RepeatMode mode) {
    mRepeatMode.store(mode);
}

RepeatMode PlaylistNavigationImpl::cycleRepeatMode() {
    RepeatMode current = mRepeatMode.load();
    RepeatMode newMode;

    switch (current) {
        case RepeatMode::NONE:
            newMode = RepeatMode::ONE;
            break;
        case RepeatMode::ONE:
            newMode = RepeatMode::ALL;
            break;
        case RepeatMode::ALL:
        default:
            newMode = RepeatMode::NONE;
            break;
    }

    mRepeatMode.store(newMode);
    return newMode;
}

bool PlaylistNavigationImpl::isShuffleEnabled() const {
    return mShuffleEnabled.load();
}

void PlaylistNavigationImpl::setShuffleEnabled(bool enabled) {
    mShuffleEnabled.store(enabled);
}

bool PlaylistNavigationImpl::toggleShuffle() {
    bool newState = !mShuffleEnabled.load();
    mShuffleEnabled.store(newState);
    return newState;
}

// ============================================================================
// Additional Methods
// ============================================================================

void PlaylistNavigationImpl::reset() {
    mTrackIndex.store(-1);
    mRepeatMode.store(RepeatMode::NONE);
    mShuffleEnabled.store(false);
}

} // namespace Model
