/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/PlayerState.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of IPlayerState using composition.
 *              Facade pattern - delegates to smaller components.
 */

#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H

#include "IPlayerState.h"
#include "playerstate/PlaybackStateImpl.h"
#include "playerstate/VolumeStateImpl.h"
#include "playerstate/TrackPositionImpl.h"
#include "playerstate/PlaylistNavigationImpl.h"
#include <memory>

namespace Model {

/**
 * @brief Thread-safe implementation of IPlayerState using composition.
 * 
 * This class acts as a facade, delegating to PlaybackStateImpl, VolumeStateImpl,
 * TrackPositionImpl, and PlaylistNavigationImpl components. It provides backward
 * compatibility with the original PlayerState interface while internally
 * following SOLID principles.
 * 
 * All operations are thread-safe as they delegate to atomic-based implementations.
 */
class PlayerState : public IPlayerState {
private:
    PlaybackStateImpl mPlaybackState;       ///< Playback state component
    VolumeStateImpl mVolumeState;           ///< Volume state component
    TrackPositionImpl mTrackPosition;       ///< Track position component
    PlaylistNavigationImpl mNavigation;     ///< Playlist navigation component

public:
    /**
     * @brief Default constructor - initializes to default state.
     */
    PlayerState();

    /**
     * @brief Destructor.
     */
    ~PlayerState() override = default;

    // Delete copy (components contain atomic members)
    PlayerState(const PlayerState&) = delete;
    PlayerState& operator=(const PlayerState&) = delete;

    // ========================================================================
    // IPlaybackState Interface Implementation (delegated)
    // ========================================================================

    PlaybackStatus getPlaybackStatus() const override;
    void setPlaybackStatus(PlaybackStatus status) override;
    bool isPlaying() const override;
    PlaybackStatus togglePlayPause() override;

    // ========================================================================
    // IVolumeState Interface Implementation (delegated)
    // ========================================================================

    int getVolume() const override;
    void setVolume(int volume) override;
    bool isMuted() const override;
    void setMuted(bool muted) override;
    bool toggleMute() override;

    // ========================================================================
    // ITrackPosition Interface Implementation (delegated)
    // ========================================================================

    uint32_t getCurrentPosition() const override;
    void setCurrentPosition(uint32_t position) override;
    uint32_t getPlaybackVersion() const override;
    void incrementPlaybackVersion() override;

    // ========================================================================
    // IPlaylistNavigation Interface Implementation (delegated)
    // ========================================================================

    int getCurrentTrackIndex() const override;
    void setCurrentTrackIndex(int index) override;
    RepeatMode getRepeatMode() const override;
    void setRepeatMode(RepeatMode mode) override;
    RepeatMode cycleRepeatMode() override;
    bool isShuffleEnabled() const override;
    void setShuffleEnabled(bool enabled) override;
    bool toggleShuffle() override;

    // ========================================================================
    // Additional Convenience Methods
    // ========================================================================

    /**
     * @brief Reset all state to defaults.
     */
    void reset();

    // ========================================================================
    // Component Access (for advanced use)
    // ========================================================================

    /**
     * @brief Get the playback state component.
     * @return Reference to PlaybackStateImpl
     */
    IPlaybackState& getPlaybackStateComponent() { return mPlaybackState; }
    const IPlaybackState& getPlaybackStateComponent() const { return mPlaybackState; }

    /**
     * @brief Get the volume state component.
     * @return Reference to VolumeStateImpl
     */
    IVolumeState& getVolumeStateComponent() { return mVolumeState; }
    const IVolumeState& getVolumeStateComponent() const { return mVolumeState; }

    /**
     * @brief Get the track position component.
     * @return Reference to TrackPositionImpl
     */
    ITrackPosition& getTrackPositionComponent() { return mTrackPosition; }
    const ITrackPosition& getTrackPositionComponent() const { return mTrackPosition; }

    /**
     * @brief Get the playlist navigation component.
     * @return Reference to PlaylistNavigationImpl
     */
    IPlaylistNavigation& getNavigationComponent() { return mNavigation; }
    const IPlaylistNavigation& getNavigationComponent() const { return mNavigation; }
};

} // namespace Model

#endif // PLAYERSTATE_H
