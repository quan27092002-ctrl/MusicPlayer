/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/TrackPositionImpl.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of TrackPositionImpl class.
 */

#include "TrackPositionImpl.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

TrackPositionImpl::TrackPositionImpl()
    : mPosition(0)
    , mPlaybackVersion(0) {
}

TrackPositionImpl::TrackPositionImpl(uint32_t position)
    : mPosition(position)
    , mPlaybackVersion(0) {
}

// ============================================================================
// ITrackPosition Interface Implementation
// ============================================================================

uint32_t TrackPositionImpl::getCurrentPosition() const {
    return mPosition.load();
}

void TrackPositionImpl::setCurrentPosition(uint32_t position) {
    mPosition.store(position);
}

uint32_t TrackPositionImpl::getPlaybackVersion() const {
    return mPlaybackVersion.load();
}

void TrackPositionImpl::incrementPlaybackVersion() {
    mPlaybackVersion.fetch_add(1);
}

// ============================================================================
// Additional Methods
// ============================================================================

void TrackPositionImpl::reset() {
    mPosition.store(0);
    mPlaybackVersion.store(0);
}

} // namespace Model

