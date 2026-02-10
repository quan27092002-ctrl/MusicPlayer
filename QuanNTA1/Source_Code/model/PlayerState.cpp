/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/PlayerState.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of PlayerState class using composition.
 */

#include "PlayerState.h"

namespace Model {

// ============================================================================
// Constructor
// ============================================================================

PlayerState::PlayerState()
    : mPlaybackState()
    , mVolumeState()
    , mTrackPosition()
    , mNavigation() {
}

// ============================================================================
// IPlaybackState Interface Implementation (delegated)
// ============================================================================

PlaybackStatus PlayerState::getPlaybackStatus() const {
    return mPlaybackState.getPlaybackStatus();
}

void PlayerState::setPlaybackStatus(PlaybackStatus status) {
    mPlaybackState.setPlaybackStatus(status);
}

bool PlayerState::isPlaying() const {
    return mPlaybackState.isPlaying();
}

PlaybackStatus PlayerState::togglePlayPause() {
    return mPlaybackState.togglePlayPause();
}

// ============================================================================
// IVolumeState Interface Implementation (delegated)
// ============================================================================

int PlayerState::getVolume() const {
    return mVolumeState.getVolume();
}

void PlayerState::setVolume(int volume) {
    mVolumeState.setVolume(volume);
}

bool PlayerState::isMuted() const {
    return mVolumeState.isMuted();
}

void PlayerState::setMuted(bool muted) {
    mVolumeState.setMuted(muted);
}

bool PlayerState::toggleMute() {
    return mVolumeState.toggleMute();
}

// ============================================================================
// ITrackPosition Interface Implementation (delegated)
// ============================================================================

uint32_t PlayerState::getCurrentPosition() const {
    return mTrackPosition.getCurrentPosition();
}

void PlayerState::setCurrentPosition(uint32_t position) {
    mTrackPosition.setCurrentPosition(position);
}

uint32_t PlayerState::getPlaybackVersion() const {
    return mTrackPosition.getPlaybackVersion();
}

void PlayerState::incrementPlaybackVersion() {
    mTrackPosition.incrementPlaybackVersion();
}

// ============================================================================
// IPlaylistNavigation Interface Implementation (delegated)
// ============================================================================

int PlayerState::getCurrentTrackIndex() const {
    return mNavigation.getCurrentTrackIndex();
}

void PlayerState::setCurrentTrackIndex(int index) {
    mNavigation.setCurrentTrackIndex(index);
}

RepeatMode PlayerState::getRepeatMode() const {
    return mNavigation.getRepeatMode();
}

void PlayerState::setRepeatMode(RepeatMode mode) {
    mNavigation.setRepeatMode(mode);
}

RepeatMode PlayerState::cycleRepeatMode() {
    return mNavigation.cycleRepeatMode();
}

bool PlayerState::isShuffleEnabled() const {
    return mNavigation.isShuffleEnabled();
}

void PlayerState::setShuffleEnabled(bool enabled) {
    mNavigation.setShuffleEnabled(enabled);
}

bool PlayerState::toggleShuffle() {
    return mNavigation.toggleShuffle();
}

// ============================================================================
// Convenience Methods
// ============================================================================

void PlayerState::reset() {
    mPlaybackState.reset();
    mVolumeState.reset();
    mTrackPosition.reset();
    mNavigation.reset();
}

} // namespace Model
