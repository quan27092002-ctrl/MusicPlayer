/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/PlayerState.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete thread-safe implementation of IPlayerState.
 */

#ifndef PLAYERSTATE_H
#define PLAYERSTATE_H

#include "IPlayerState.h"
#include <atomic>
#include <mutex>
#include <cstdint>

namespace Model {

/**
 * @brief Thread-safe implementation of IPlayerState.
 * 
 * Uses std::atomic for simple types and std::mutex for complex operations.
 * All public methods are safe to call from multiple threads.
 */
class PlayerState : public IPlayerState {
private:
    std::atomic<PlaybackState> mPlaybackState;  ///< Current playback state
    std::atomic<int> mVolume;                    ///< Volume level (0-100)
    std::atomic<bool> mMuted;                    ///< Mute flag
    std::atomic<uint32_t> mCurrentPosition;      ///< Current position in seconds
    std::atomic<int> mCurrentTrackIndex;         ///< Current track index (-1 if none)
    std::atomic<RepeatMode> mRepeatMode;         ///< Repeat mode
    std::atomic<bool> mShuffleEnabled;           ///< Shuffle flag

    // Volume limits
    static constexpr int MIN_VOLUME = 0;
    static constexpr int MAX_VOLUME = 100;

    // Helper to clamp volume
    int clampVolume(int volume) const;

public:
    /**
     * @brief Default constructor - initializes to default state.
     */
    PlayerState();

    /**
     * @brief Destructor.
     */
    ~PlayerState() override = default;

    // Delete copy (atomic members are not copyable)
    PlayerState(const PlayerState&) = delete;
    PlayerState& operator=(const PlayerState&) = delete;

    // ========================================================================
    // IPlayerState Interface Implementation
    // ========================================================================

    // Playback State
    PlaybackState getPlaybackState() const override;
    void setPlaybackState(PlaybackState state) override;
    bool isPlaying() const override;

    // Volume Control
    int getVolume() const override;
    void setVolume(int volume) override;
    bool isMuted() const override;
    void setMuted(bool muted) override;

    // Track Position
    uint32_t getCurrentPosition() const override;
    void setCurrentPosition(uint32_t position) override;

    // Playlist Navigation
    int getCurrentTrackIndex() const override;
    void setCurrentTrackIndex(int index) override;

    // Playback Modes
    RepeatMode getRepeatMode() const override;
    void setRepeatMode(RepeatMode mode) override;
    bool isShuffleEnabled() const override;
    void setShuffleEnabled(bool enabled) override;

    // ========================================================================
    // Additional Convenience Methods
    // ========================================================================

    /**
     * @brief Reset state to defaults.
     */
    void reset();

    /**
     * @brief Toggle play/pause state.
     * @return New playback state after toggle
     */
    PlaybackState togglePlayPause();

    /**
     * @brief Toggle mute state.
     * @return New mute state after toggle
     */
    bool toggleMute();

    /**
     * @brief Cycle through repeat modes (NONE -> ONE -> ALL -> NONE).
     * @return New repeat mode after cycle
     */
    RepeatMode cycleRepeatMode();

    /**
     * @brief Toggle shuffle mode.
     * @return New shuffle state after toggle
     */
    bool toggleShuffle();
};

} // namespace Model

#endif // PLAYERSTATE_H
