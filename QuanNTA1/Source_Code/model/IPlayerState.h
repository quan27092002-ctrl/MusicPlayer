/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/IPlayerState.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Aggregate interface for PlayerState - combines IPlaybackState,
 *              IVolumeState, ITrackPosition, and IPlaylistNavigation interfaces.
 *              Provides backward compatibility while following ISP.
 */

#ifndef IPLAYERSTATE_H
#define IPLAYERSTATE_H

#include "playerstate/interfaces/IPlaybackState.h"
#include "playerstate/interfaces/IVolumeState.h"
#include "playerstate/interfaces/ITrackPosition.h"
#include "playerstate/interfaces/IPlaylistNavigation.h"

namespace Model {

// Re-export PlaybackStatus and RepeatMode for backward compatibility
// (They are now defined in the smaller interfaces)
using PlaybackState = PlaybackStatus;

/**
 * @brief Aggregate interface for player state.
 * 
 * This interface combines IPlaybackState, IVolumeState, ITrackPosition,
 * and IPlaylistNavigation into a single interface for backward compatibility.
 * It allows clients that need all player state features to depend on this
 * single interface, while clients that only need specific features can depend
 * on the smaller interfaces directly (Interface Segregation Principle).
 * 
 * This interface is thread-safe - all implementations must ensure
 * thread safety for concurrent access.
 */
class IPlayerState : public IPlaybackState,
                     public IVolumeState,
                     public ITrackPosition,
                     public IPlaylistNavigation {
public:
    virtual ~IPlayerState() = default;

    // ========================================================================
    // Backward Compatibility Methods
    // ========================================================================

    /**
     * @brief Get the current playback state (alias for getPlaybackStatus).
     * @return Current PlaybackStatus (STOPPED, PLAYING, PAUSED)
     * @deprecated Use getPlaybackStatus() instead
     */
    virtual PlaybackStatus getPlaybackState() const {
        return getPlaybackStatus();
    }

    /**
     * @brief Set the playback state (alias for setPlaybackStatus).
     * @param state New playback state
     * @deprecated Use setPlaybackStatus() instead
     */
    virtual void setPlaybackState(PlaybackStatus state) {
        setPlaybackStatus(state);
    }
};

} // namespace Model

#endif // IPLAYERSTATE_H
