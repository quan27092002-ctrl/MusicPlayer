/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/controller/MockAppController.h
 * DESCRIPTION: GoogleMock implementation for IAppController interface.
 */

#ifndef MOCK_APP_CONTROLLER_H
#define MOCK_APP_CONTROLLER_H

#include <gmock/gmock.h>
#include "controller/IAppController.h"

namespace Controller {

class MockAppController : public IAppController {
public:
    // ========================================================================
    // IAppLifecycle
    // ========================================================================
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(AppState, getState, (), (const, override));
    MOCK_METHOD(void, setStateCallback, (AppStateCallback callback), (override));

    // ========================================================================
    // IPlaybackController
    // ========================================================================
    MOCK_METHOD(bool, loadTrack, (const std::string& filePath), (override));
    MOCK_METHOD(void, play, (), (override));
    MOCK_METHOD(void, pause, (), (override));
    MOCK_METHOD(void, stop, (), (override));
    MOCK_METHOD(void, next, (), (override));
    MOCK_METHOD(void, previous, (), (override));
    MOCK_METHOD(void, playTrack, (int index), (override));
    MOCK_METHOD(void, seek, (uint32_t positionMs), (override));
    MOCK_METHOD(void, queueNext, (const std::string& filePath), (override));
    MOCK_METHOD(void, replaceQueue, (const std::vector<std::string>& filePaths), (override));
    MOCK_METHOD(void, queuePlaylist, (const std::vector<std::string>& filePaths), (override));
    MOCK_METHOD(void, playLibrary, (int startIndex), (override));

    // ========================================================================
    // IVolumeController
    // ========================================================================
    MOCK_METHOD(void, setVolume, (int volume), (override));
    MOCK_METHOD(int, getVolume, (), (const, override));
    MOCK_METHOD(void, toggleMute, (), (override));

    // ========================================================================
    // IPlaylistManager
    // ========================================================================
    MOCK_METHOD(void, addToPlaylist, (const std::string& filePath), (override));
    MOCK_METHOD(void, clearPlaylist, (), (override));
    MOCK_METHOD(size_t, getPlaylistSize, (), (const, override));
    MOCK_METHOD(size_t, loadDirectory, (const std::string& directoryPath), (override));
    MOCK_METHOD(std::string, getTrackName, (size_t index), (const, override));
    MOCK_METHOD(std::string, getTrackPath, (size_t index), (const, override));
    MOCK_METHOD(std::string, getTrackArtist, (size_t index), (const, override));
    MOCK_METHOD(std::string, getTrackAlbum, (size_t index), (const, override));
    MOCK_METHOD(uint32_t, getTrackDuration, (size_t index), (const, override));
    MOCK_METHOD(std::vector<uint8_t>, getTrackCoverArt, (size_t index), (const, override));
    MOCK_METHOD(std::shared_ptr<Model::MediaFile>, acquireMediaFile, (const std::string& filePath), (override));
    MOCK_METHOD(size_t, getLibrarySize, (), (const, override));
    MOCK_METHOD(std::string, getLibraryTrackName, (size_t index), (const, override));
    MOCK_METHOD(std::string, getLibraryTrackPath, (size_t index), (const, override));
    MOCK_METHOD(std::string, getLibraryTrackArtist, (size_t index), (const, override));
    MOCK_METHOD(std::string, getLibraryTrackAlbum, (size_t index), (const, override));
    MOCK_METHOD(std::vector<uint8_t>, getLibraryTrackCoverArt, (size_t index), (const, override));
    MOCK_METHOD(void, setPlaylistUpdatedCallback, (std::function<void()> callback), (override));
    MOCK_METHOD(void, notifyPlaylistUpdated, (), (override));

    // ========================================================================
    // IHistoryManager
    // ========================================================================
    MOCK_METHOD(std::vector<int>, getHistory, (), (const, override));
    MOCK_METHOD(size_t, getHistorySize, (), (const, override));
    MOCK_METHOD(std::string, getHistoryTrackPath, (size_t index), (const, override));
    MOCK_METHOD(std::shared_ptr<Model::MediaFile>, getHistoryItem, (size_t index), (const, override));
    MOCK_METHOD(std::string, getHistoryTrackName, (size_t index), (const, override));
    MOCK_METHOD(std::string, getHistoryTrackArtist, (size_t index), (const, override));
    MOCK_METHOD(std::string, getHistoryTrackAlbum, (size_t index), (const, override));
    MOCK_METHOD(std::vector<uint8_t>, getHistoryTrackCoverArt, (size_t index), (const, override));
    MOCK_METHOD(void, playHistoryTrack, (size_t index), (override));

    // ========================================================================
    // IBoardCommunicator
    // ========================================================================
    MOCK_METHOD(bool, connectToBoard, (const std::string& portName, uint32_t baudRate), (override));
    MOCK_METHOD(void, disconnectFromBoard, (), (override));
    MOCK_METHOD(bool, isConnectedToBoard, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, getAvailablePorts, (), (const, override));

    // ========================================================================
    // IAppController specific
    // ========================================================================
    MOCK_METHOD(void, playPlaylist, (const std::vector<std::string>& filePaths), (override));
    MOCK_METHOD(std::vector<StorageDevice>, getStorageDevices, (), (override));
    MOCK_METHOD(size_t, loadFromStorage, (const std::string& path), (override));
    MOCK_METHOD((std::pair<size_t, size_t>), getLoadingProgress, (), (const, override));
    MOCK_METHOD(void, toggleShuffle, (), (override));
    MOCK_METHOD(void, toggleRepeat, (), (override));
};

} // namespace Controller

#endif // MOCK_APP_CONTROLLER_H
