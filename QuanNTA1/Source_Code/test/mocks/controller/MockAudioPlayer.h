/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/controller/MockAudioPlayer.h
 * DESCRIPTION: GoogleMock implementation for IAudioPlayer interface.
 */

#ifndef MOCK_AUDIO_PLAYER_H
#define MOCK_AUDIO_PLAYER_H

#include <gmock/gmock.h>
#include "controller/IAudioPlayer.h"

namespace Controller {

class MockAudioPlayer : public IAudioPlayer {
public:
    // ========================================================================
    // IAudioLifecycle
    // ========================================================================
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(AudioState, getState, (), (const, override));
    MOCK_METHOD(void, setCallback, (AudioCallback callback), (override));

    // ========================================================================
    // IAudioLoader
    // ========================================================================
    MOCK_METHOD(bool, load, (const std::string& filePath), (override));
    MOCK_METHOD(void, unload, (), (override));
    MOCK_METHOD(bool, isLoaded, (), (const, override));

    // ========================================================================
    // IAudioPlayback
    // ========================================================================
    MOCK_METHOD(void, play, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, seek, (uint32_t positionMs), (override));
    MOCK_METHOD(bool, isPlaying, (), (const, override));
    MOCK_METHOD(uint32_t, getPosition, (), (const, override));
    MOCK_METHOD(uint32_t, getDuration, (), (const, override));
    MOCK_METHOD(void, setFinishedCallback, (std::function<void()> callback), (override));

    // ========================================================================
    // IAudioVolume
    // ========================================================================
    MOCK_METHOD(void, setVolume, (int volume), (override));
    MOCK_METHOD(int, getVolume, (), (const, override));
};

} // namespace Controller

#endif // MOCK_AUDIO_PLAYER_H
