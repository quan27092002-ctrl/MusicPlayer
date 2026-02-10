/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/view/MockAssetManager.h
 * DESCRIPTION: GoogleMock implementation for IAssetManager interface.
 */

#ifndef MOCK_ASSET_MANAGER_H
#define MOCK_ASSET_MANAGER_H

#include <gmock/gmock.h>
#include "view/imguiview/interfaces/IAssetManager.h"

namespace View {

class MockAssetManager : public IAssetManager {
public:
    MOCK_METHOD(void, clearCache, (), (override));
    MOCK_METHOD(void, drawAlbumCover, (ImDrawList* drawList, ImVec2 pos, float size,
                const std::string& cacheKey, const std::vector<uint8_t>& coverData), (override));
};

} // namespace View

#endif // MOCK_ASSET_MANAGER_H
