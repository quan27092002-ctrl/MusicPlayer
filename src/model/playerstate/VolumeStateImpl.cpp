/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/VolumeStateImpl.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of VolumeStateImpl class.
 */

#include "VolumeStateImpl.h"
#include <algorithm>

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

VolumeStateImpl::VolumeStateImpl()
    : mVolume(DEFAULT_VOLUME)
    , mMuted(false) {
}

VolumeStateImpl::VolumeStateImpl(int volume, bool muted)
    : mVolume(clampVolume(volume))
    , mMuted(muted) {
}

// ============================================================================
// Private Helper
// ============================================================================

int VolumeStateImpl::clampVolume(int volume) const {
    return std::clamp(volume, MIN_VOLUME, MAX_VOLUME);
}

// ============================================================================
// IVolumeState Interface Implementation
// ============================================================================

int VolumeStateImpl::getVolume() const {
    return mVolume.load();
}

void VolumeStateImpl::setVolume(int volume) {
    mVolume.store(clampVolume(volume));
}

bool VolumeStateImpl::isMuted() const {
    return mMuted.load();
}

void VolumeStateImpl::setMuted(bool muted) {
    mMuted.store(muted);
}

bool VolumeStateImpl::toggleMute() {
    bool newState = !mMuted.load();
    mMuted.store(newState);
    return newState;
}

// ============================================================================
// Additional Methods
// ============================================================================

void VolumeStateImpl::reset() {
    mVolume.store(DEFAULT_VOLUME);
    mMuted.store(false);
}

} // namespace Model
