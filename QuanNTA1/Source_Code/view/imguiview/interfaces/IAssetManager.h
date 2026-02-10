/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/interfaces/IAssetManager.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for asset management (textures, covers).
 */

#ifndef IASSETMANAGER_H
#define IASSETMANAGER_H

#include <cstdint>
#include <string>
#include <vector>

struct ImDrawList;
struct ImVec2;

namespace View {

/**
 * @brief Interface for managing texture assets.
 */
class IAssetManager {
public:
    virtual ~IAssetManager() = default;

    virtual void clearCache() = 0;
    virtual void drawAlbumCover(ImDrawList* drawList,
                                 ImVec2 pos,
                                 float size,
                                 const std::string& cacheKey,
                                 const std::vector<uint8_t>& coverData) = 0;
};

} // namespace View

#endif // IASSETMANAGER_H
