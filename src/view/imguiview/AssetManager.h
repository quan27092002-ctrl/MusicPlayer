/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/AssetManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Manages texture assets (ISP/SRP).
 */

#ifndef ASSETMANAGER_H
#define ASSETMANAGER_H

#include <SDL2/SDL.h>
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include "../imgui/imgui.h"

namespace View {

class AssetManager {
public:
    AssetManager(SDL_Renderer* renderer);
    ~AssetManager();

    void clearCache();
    
    /**
     * @brief Draw album cover at specified position.
     * Uses internal cache to avoid recreating textures.
     */
    void drawAlbumCover(ImDrawList* dl, ImVec2 pos, float size, const std::string& cacheKey, const std::vector<uint8_t>& data);

private:
    SDL_Renderer* mRenderer;
    std::map<std::string, SDL_Texture*> mCoverCache; // CacheKey -> Texture
    
    SDL_Texture* createTextureFromMemory(const std::vector<uint8_t>& data);
};

} // namespace View

#endif // ASSETMANAGER_H
