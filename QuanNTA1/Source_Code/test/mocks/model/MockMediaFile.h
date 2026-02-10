/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/model/MockMediaFile.h
 * DESCRIPTION: GoogleMock implementation for IMediaFile interface.
 */

#ifndef MOCK_MEDIA_FILE_H
#define MOCK_MEDIA_FILE_H

#include <gmock/gmock.h>
#include "model/IMediaFile.h"

namespace Model {

class MockMediaFile : public IMediaFile {
public:
    // ========================================================================
    // IMediaFileInfo
    // ========================================================================
    MOCK_METHOD(std::string, getFilename, (), (const, override));
    MOCK_METHOD(void, setFilename, (const std::string& filename), (override));
    MOCK_METHOD(std::string, getPath, (), (const, override));
    MOCK_METHOD(void, setPath, (const std::string& path), (override));
    MOCK_METHOD(bool, isValid, (), (const, override));

    // ========================================================================
    // IMediaMetadata
    // ========================================================================
    MOCK_METHOD(uint32_t, getDuration, (), (const, override));
    MOCK_METHOD(void, setDuration, (uint32_t duration), (override));
    MOCK_METHOD(std::string, getArtist, (), (const, override));
    MOCK_METHOD(void, setArtist, (const std::string& artist), (override));
    MOCK_METHOD(std::string, getAlbum, (), (const, override));
    MOCK_METHOD(void, setAlbum, (const std::string& album), (override));

    // ========================================================================
    // ICoverArt
    // ========================================================================
    MOCK_METHOD(const std::vector<uint8_t>&, getCoverArt, (), (const, override));
    MOCK_METHOD(void, setCoverArt, (const std::vector<uint8_t>& data), (override));
    MOCK_METHOD(bool, hasCoverArt, (), (const, override));
};

} // namespace Model

#endif // MOCK_MEDIA_FILE_H
