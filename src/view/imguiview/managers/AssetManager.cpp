/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/AssetManager.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of AssetManager.
 */

#include "AssetManager.h"
#include <SDL2/SDL_image.h>
#include <cmath>

namespace View {

AssetManager::AssetManager(SDL_Renderer* renderer)
    : mRenderer(renderer)
{}

AssetManager::~AssetManager() {
    clearCache();
}

void AssetManager::clearCache() {
    for (auto& pair : mCoverCache) {
        if (pair.second) {
            SDL_DestroyTexture(pair.second);
        }
    }
    mCoverCache.clear();
}

SDL_Texture* AssetManager::createTextureFromMemory(const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    SDL_RWops* rw = SDL_RWFromConstMem(data.data(), data.size());
    if (!rw) return nullptr;
    
    SDL_Surface* surface = IMG_Load_RW(rw, 1);
    if (!surface) return nullptr;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void AssetManager::drawAlbumCover(ImDrawList* dl, ImVec2 pos, float size, const std::string& cacheKey, const std::vector<uint8_t>& data) {
    bool drawn = false;
    
    // Check or load cache
    SDL_Texture* texture = nullptr;
    auto it = mCoverCache.find(cacheKey);
    
    if (it != mCoverCache.end()) {
        texture = it->second;
    } else {
        if (!data.empty()) {
            texture = createTextureFromMemory(data);
            if (texture) {
                mCoverCache[cacheKey] = texture;
            }
        }
    }
    
    if (texture) {
        dl->AddImage((ImTextureID)texture, pos, ImVec2(pos.x + size, pos.y + size));
        drawn = true;
    }

    if (!drawn) {
        // Fallback placeholder pattern
        // Generate pseudo-random index from string hash
        std::size_t hash = std::hash<std::string>{}(cacheKey);
        int colorIndex = (int)hash;
        // Spotify-like gradients approximation
        ImU32 colors1[] = {
            IM_COL32(180, 100, 60, 255), IM_COL32(100, 80, 180, 255),
            IM_COL32(60, 150, 120, 255), IM_COL32(180, 60, 100, 255),
            IM_COL32(80, 120, 180, 255), IM_COL32(150, 120, 80, 255),
        };
        ImU32 colors2[] = {
            IM_COL32(80, 40, 30, 255), IM_COL32(40, 30, 80, 255),
            IM_COL32(30, 70, 50, 255), IM_COL32(80, 30, 50, 255),
            IM_COL32(30, 50, 80, 255), IM_COL32(60, 50, 30, 255),
        };
        
        int idx = std::abs(colorIndex) % 6;
        dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + size, pos.y + size),
            colors1[idx], colors1[idx], colors2[idx], colors2[idx]);
        dl->AddRect(pos, ImVec2(pos.x + size, pos.y + size), 
            IM_COL32(255, 255, 255, 20), 4.0f, 0, 1.0f);
        
        // Simple circle note
        dl->AddCircle(ImVec2(pos.x + size/2, pos.y + size/2), size/3, IM_COL32(255,255,255,100), 32, 2.0f);
    }
}

} // namespace View
