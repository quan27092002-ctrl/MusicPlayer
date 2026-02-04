/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/TrackPositionImpl.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of TrackPositionImpl class.
 */

#include "TrackPositionImpl.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

TrackPositionImpl::TrackPositionImpl()
    : mPosition(0) {
}

TrackPositionImpl::TrackPositionImpl(uint32_t position)
    : mPosition(position) {
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

// ============================================================================
// Additional Methods
// ============================================================================

void TrackPositionImpl::reset() {
    mPosition.store(0);
}

} // namespace Model
