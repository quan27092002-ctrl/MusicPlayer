/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/PlaylistNavigationImpl.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Thread-safe concrete implementation of IPlaylistNavigation.
 */

#ifndef PLAYLISTNAVIGATIONIMPL_H
#define PLAYLISTNAVIGATIONIMPL_H

#include "interfaces/IPlaylistNavigation.h"
#include <atomic>

namespace Model {

/**
 * @brief Thread-safe implementation of IPlaylistNavigation.
 * 
 * Uses std::atomic for thread-safe access to playlist navigation state.
 * Follows Single Responsibility Principle (SRP).
 */
class PlaylistNavigationImpl : public IPlaylistNavigation {
private:
    std::atomic<int> mTrackIndex;          ///< Current track index (-1 if none)
    std::atomic<RepeatMode> mRepeatMode;   ///< Repeat mode
    std::atomic<bool> mShuffleEnabled;     ///< Shuffle flag

public:
    /**
     * @brief Default constructor - initializes to default state.
     */
    PlaylistNavigationImpl();

    /**
     * @brief Parameterized constructor.
     * @param trackIndex Initial track index
     * @param repeatMode Initial repeat mode
     * @param shuffleEnabled Initial shuffle state
     */
    PlaylistNavigationImpl(int trackIndex, 
                           RepeatMode repeatMode = RepeatMode::NONE, 
                           bool shuffleEnabled = false);

    /**
     * @brief Destructor.
     */
    ~PlaylistNavigationImpl() override = default;

    // Delete copy (atomic members are not copyable)
    PlaylistNavigationImpl(const PlaylistNavigationImpl&) = delete;
    PlaylistNavigationImpl& operator=(const PlaylistNavigationImpl&) = delete;

    // ========================================================================
    // IPlaylistNavigation Interface Implementation
    // ========================================================================

    int getCurrentTrackIndex() const override;
    void setCurrentTrackIndex(int index) override;
    RepeatMode getRepeatMode() const override;
    void setRepeatMode(RepeatMode mode) override;
    RepeatMode cycleRepeatMode() override;
    bool isShuffleEnabled() const override;
    void setShuffleEnabled(bool enabled) override;
    bool toggleShuffle() override;

    /**
     * @brief Reset to default state.
     */
    void reset();
};

} // namespace Model

#endif // PLAYLISTNAVIGATIONIMPL_H
