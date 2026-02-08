/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/model/MockPlayerState.h
 * DESCRIPTION: GoogleMock implementation for IPlayerState interface.
 */

#ifndef MOCK_PLAYER_STATE_H
#define MOCK_PLAYER_STATE_H

#include <gmock/gmock.h>
#include "model/IPlayerState.h"

namespace Model {

class MockPlayerState : public IPlayerState {
public:
    // ========================================================================
    // IPlaybackState
    // ========================================================================
    MOCK_METHOD(PlaybackStatus, getPlaybackStatus, (), (const, override));
    MOCK_METHOD(void, setPlaybackStatus, (PlaybackStatus status), (override));
    MOCK_METHOD(bool, isPlaying, (), (const, override));
    MOCK_METHOD(PlaybackStatus, togglePlayPause, (), (override));

    // ========================================================================
    // IVolumeState
    // ========================================================================
    MOCK_METHOD(int, getVolume, (), (const, override));
    MOCK_METHOD(void, setVolume, (int volume), (override));
    MOCK_METHOD(bool, isMuted, (), (const, override));
    MOCK_METHOD(void, setMuted, (bool muted), (override));
    MOCK_METHOD(bool, toggleMute, (), (override));

    // ========================================================================
    // ITrackPosition
    // ========================================================================
    MOCK_METHOD(uint32_t, getCurrentPosition, (), (const, override));
    MOCK_METHOD(void, setCurrentPosition, (uint32_t position), (override));
    MOCK_METHOD(uint32_t, getPlaybackVersion, (), (const, override));
    MOCK_METHOD(void, incrementPlaybackVersion, (), (override));

    // ========================================================================
    // IPlaylistNavigation
    // ========================================================================
    MOCK_METHOD(int, getCurrentTrackIndex, (), (const, override));
    MOCK_METHOD(void, setCurrentTrackIndex, (int index), (override));
    MOCK_METHOD(RepeatMode, getRepeatMode, (), (const, override));
    MOCK_METHOD(void, setRepeatMode, (RepeatMode mode), (override));
    MOCK_METHOD(RepeatMode, cycleRepeatMode, (), (override));
    MOCK_METHOD(bool, isShuffleEnabled, (), (const, override));
    MOCK_METHOD(void, setShuffleEnabled, (bool enabled), (override));
    MOCK_METHOD(bool, toggleShuffle, (), (override));
};

} // namespace Model

#endif // MOCK_PLAYER_STATE_H
