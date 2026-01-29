/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/PlayerState.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of PlayerState class.
 */

#include "PlayerState.h"
#include <algorithm>

namespace Model {

// ============================================================================
// Constructor
// ============================================================================

PlayerState::PlayerState()
    : mPlaybackState(PlaybackState::STOPPED)
    , mVolume(50)
    , mMuted(false)
    , mCurrentPosition(0)
    , mCurrentTrackIndex(-1)
    , mRepeatMode(RepeatMode::NONE)
    , mShuffleEnabled(false) {
}

// ============================================================================
// Private Helper
// ============================================================================

int PlayerState::clampVolume(int volume) const {
    return std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
}

// ============================================================================
// Playback State
// ============================================================================

PlaybackState PlayerState::getPlaybackState() const {
    return mPlaybackState.load();
}

void PlayerState::setPlaybackState(PlaybackState state) {
    mPlaybackState.store(state);
}

bool PlayerState::isPlaying() const {
    return mPlaybackState.load() == PlaybackState::PLAYING;
}

// ============================================================================
// Volume Control
// ============================================================================

int PlayerState::getVolume() const {
    return mVolume.load();
}

void PlayerState::setVolume(int volume) {
    mVolume.store(clampVolume(volume));
}

bool PlayerState::isMuted() const {
    return mMuted.load();
}

void PlayerState::setMuted(bool muted) {
    mMuted.store(muted);
}

// ============================================================================
// Track Position
// ============================================================================

uint32_t PlayerState::getCurrentPosition() const {
    return mCurrentPosition.load();
}

void PlayerState::setCurrentPosition(uint32_t position) {
    mCurrentPosition.store(position);
}

// ============================================================================
// Playlist Navigation
// ============================================================================

int PlayerState::getCurrentTrackIndex() const {
    return mCurrentTrackIndex.load();
}

void PlayerState::setCurrentTrackIndex(int index) {
    mCurrentTrackIndex.store(index);
}

// ============================================================================
// Playback Modes
// ============================================================================

RepeatMode PlayerState::getRepeatMode() const {
    return mRepeatMode.load();
}

void PlayerState::setRepeatMode(RepeatMode mode) {
    mRepeatMode.store(mode);
}

bool PlayerState::isShuffleEnabled() const {
    return mShuffleEnabled.load();
}

void PlayerState::setShuffleEnabled(bool enabled) {
    mShuffleEnabled.store(enabled);
}

// ============================================================================
// Convenience Methods
// ============================================================================

void PlayerState::reset() {
    mPlaybackState.store(PlaybackState::STOPPED);
    mVolume.store(50);
    mMuted.store(false);
    mCurrentPosition.store(0);
    mCurrentTrackIndex.store(-1);
    mRepeatMode.store(RepeatMode::NONE);
    mShuffleEnabled.store(false);
}

PlaybackState PlayerState::togglePlayPause() {
    PlaybackState current = mPlaybackState.load();
    PlaybackState newState;

    if (current == PlaybackState::PLAYING) {
        newState = PlaybackState::PAUSED;
    } else {
        newState = PlaybackState::PLAYING;
    }

    mPlaybackState.store(newState);
    return newState;
}

bool PlayerState::toggleMute() {
    bool newState = !mMuted.load();
    mMuted.store(newState);
    return newState;
}

RepeatMode PlayerState::cycleRepeatMode() {
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

bool PlayerState::toggleShuffle() {
    bool newState = !mShuffleEnabled.load();
    mShuffleEnabled.store(newState);
    return newState;
}

} // namespace Model
