/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/PlaybackStateImpl.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Thread-safe concrete implementation of IPlaybackState.
 */

#ifndef PLAYBACKSTATEIMPL_H
#define PLAYBACKSTATEIMPL_H

#include "interfaces/IPlaybackState.h"
#include <atomic>

namespace Model {

/**
 * @brief Thread-safe implementation of IPlaybackState.
 * 
 * Uses std::atomic for thread-safe access to playback state.
 * Follows Single Responsibility Principle (SRP).
 */
class PlaybackStateImpl : public IPlaybackState {
private:
    std::atomic<PlaybackStatus> mStatus;  ///< Current playback status

public:
    /**
     * @brief Default constructor - initializes to STOPPED.
     */
    PlaybackStateImpl();

    /**
     * @brief Parameterized constructor.
     * @param status Initial playback status
     */
    explicit PlaybackStateImpl(PlaybackStatus status);

    /**
     * @brief Destructor.
     */
    ~PlaybackStateImpl() override = default;

    // Delete copy (atomic members are not copyable)
    PlaybackStateImpl(const PlaybackStateImpl&) = delete;
    PlaybackStateImpl& operator=(const PlaybackStateImpl&) = delete;

    // ========================================================================
    // IPlaybackState Interface Implementation
    // ========================================================================

    PlaybackStatus getPlaybackStatus() const override;
    void setPlaybackStatus(PlaybackStatus status) override;
    bool isPlaying() const override;
    PlaybackStatus togglePlayPause() override;

    /**
     * @brief Reset to default state (STOPPED).
     */
    void reset();
};

} // namespace Model

#endif // PLAYBACKSTATEIMPL_H
