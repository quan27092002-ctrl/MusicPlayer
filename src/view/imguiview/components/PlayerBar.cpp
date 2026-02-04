/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/PlayerBar.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of PlayerBar.
 */

#include "PlayerBar.h"
#include "../../imgui/imgui.h"
#include <algorithm>

namespace View {

// Minimal Color Palette Reuse
namespace Colors {
    const ImU32 Black       = IM_COL32(0, 0, 0, 255);
    const ImU32 White       = IM_COL32(255, 255, 255, 255);
    const ImU32 TextSecondary = IM_COL32(179, 179, 179, 255);
    const ImU32 TextMuted   = IM_COL32(115, 115, 115, 255);
    const ImVec4 BlackV     = ImVec4(0, 0, 0, 1);
    const ImVec4 TextMutedV = ImVec4(0.45f, 0.45f, 0.45f, 1);
    const ImVec4 HoverV     = ImVec4(0.24f, 0.24f, 0.24f, 1);
    const ImVec4 WhiteV     = ImVec4(1, 1, 1, 1);
}

PlayerBar::PlayerBar(std::shared_ptr<Controller::IAppController> controller,
                     std::shared_ptr<Model::IPlayerState> playerState,
                     std::shared_ptr<AssetManager> assetManager)
    : mController(controller)
    , mPlayerState(playerState)
    , mAssetManager(assetManager)
    , mWasPlaying(false)
    , mPlayStartPos(0)
    , mLastTrack(-1)
    , mIsDraggingSlider(false)
{}

std::string PlayerBar::stripExtension(const std::string& name) {
    if (name.length() > 4 && name.substr(name.length()-4) == ".mp3")
        return name.substr(0, name.length() - 4);
    if (name.length() > 4 && name.substr(name.length()-4) == ".wav")
        return name.substr(0, name.length() - 4);
    return name;
}

void PlayerBar::update(float windowWidth, float windowHeight) {
    (void)windowWidth;
    (void)windowHeight;
    // Logic updates if needed per frame before render
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    bool isPlaying = mPlayerState ? mPlayerState->isPlaying() : false;

    if (currentTrack != mLastTrack) {
        mPlayStartTime = std::chrono::steady_clock::now();
        mPlayStartPos = 0;
        mLastTrack = currentTrack;
    }

    if (!isPlaying && mWasPlaying) {
         auto now = std::chrono::steady_clock::now();
         mPlayStartPos += (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - mPlayStartTime).count();
    }

    if (isPlaying && !mWasPlaying) {
        mPlayStartTime = std::chrono::steady_clock::now();
    }
    mWasPlaying = isPlaying;
}

void PlayerBar::render() {
    int mWindowWidth = ImGui::GetIO().DisplaySize.x;
    int mWindowHeight = ImGui::GetIO().DisplaySize.y;
    const float playerBarH = 90.0f;
    
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    bool isPlaying = mPlayerState ? mPlayerState->isPlaying() : false;

    ImGui::SetNextWindowPos(ImVec2(0, mWindowHeight - playerBarH));
    ImGui::SetNextWindowSize(ImVec2((float)mWindowWidth, playerBarH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::BlackV);
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

    ImGui::Begin("PlayerBar", nullptr, flags);
    ImDrawList* pdl = ImGui::GetWindowDrawList();
    ImVec2 barPos = ImGui::GetWindowPos();
    
    // === LEFT: Album Art & Info ===
    ImVec2 coverPos = ImVec2(barPos.x + 15, barPos.y + 13);
    
    std::vector<uint8_t> artData;
    if (mController && currentTrack >= 0) {
        artData = mController->getTrackCoverArt(currentTrack);
    }
    
    mAssetManager->drawAlbumCover(pdl, coverPos, 64, currentTrack >= 0 ? currentTrack : 0, artData);
    
    // Text Info
    float infoX = coverPos.x + 75;
    float centerX = mWindowWidth / 2.0f;
    
    std::string pTitle = "No Track";
    std::string pArtist = "Unknown Artist";
    std::string pAlbum = "Unknown Album";
    if (mController && currentTrack >= 0) {
        pTitle = stripExtension(mController->getTrackName(currentTrack));
        pArtist = mController->getTrackArtist(currentTrack);
        pAlbum = mController->getTrackAlbum(currentTrack);
    }
    
    // Title
    ImGui::SetCursorPos(ImVec2(infoX, 18));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%s", pTitle.c_str());
    ImGui::PopFont();
    
    // Artist
    ImGui::SetCursorPos(ImVec2(infoX, 38));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%s", pArtist.c_str());
    
    // Album
    ImGui::SetCursorPos(ImVec2(infoX, 53));
    ImGui::Text("%s", pAlbum.c_str());
    ImGui::PopStyleColor();

    // === CENTER: Controls & Progress ===
    float controlsY = 20.0f;
    float buttonCenterY = controlsY + 18.0f; // Center of button (20 + 36/2)
    
    // Play/Pause Center
    {
        ImVec2 centerBtn = ImVec2(barPos.x + centerX, barPos.y + buttonCenterY);
        bool hovered = ImGui::IsMouseHoveringRect(
            ImVec2(centerBtn.x - 18, centerBtn.y - 18), 
            ImVec2(centerBtn.x + 18, centerBtn.y + 18));
            
        pdl->AddCircleFilled(centerBtn, 18, hovered ? Colors::TextSecondary : Colors::White, 32);
        
        // Icon
        if (isPlaying) {
             pdl->AddRectFilled(ImVec2(centerBtn.x - 5, centerBtn.y - 6), 
                               ImVec2(centerBtn.x - 1, centerBtn.y + 6), Colors::Black);
             pdl->AddRectFilled(ImVec2(centerBtn.x + 1, centerBtn.y - 6), 
                               ImVec2(centerBtn.x + 5, centerBtn.y + 6), Colors::Black);
        } else {
             pdl->AddTriangleFilled(
                 ImVec2(centerBtn.x - 4, centerBtn.y - 7),
                 ImVec2(centerBtn.x - 4, centerBtn.y + 7),
                 ImVec2(centerBtn.x + 7, centerBtn.y),
                 Colors::Black);
        }
        
        ImGui::SetCursorPos(ImVec2(centerX - 18, controlsY));
        if (ImGui::InvisibleButton("##play", ImVec2(36, 36)) && mController) {
            isPlaying ? mController->pause() : mController->play();
        }
    }
    
    // Prev Button
    {
        ImVec2 prevPos = ImVec2(barPos.x + centerX - 50, barPos.y + buttonCenterY);
        pdl->AddTriangleFilled(
            ImVec2(prevPos.x + 6, prevPos.y - 6),
            ImVec2(prevPos.x + 6, prevPos.y + 6),
            ImVec2(prevPos.x - 4, prevPos.y), Colors::TextSecondary);
        pdl->AddRectFilled(ImVec2(prevPos.x - 8, prevPos.y - 6),
                          ImVec2(prevPos.x - 6, prevPos.y + 6), Colors::TextSecondary);
                          
        ImGui::SetCursorPos(ImVec2(centerX - 65, controlsY)); // 50 + 15 = 65
        if (ImGui::InvisibleButton("##prev", ImVec2(30, 30)) && mController) mController->previous();
    }

    // Next Button
    {
         ImVec2 nextPos = ImVec2(barPos.x + centerX + 50, barPos.y + buttonCenterY);
         pdl->AddTriangleFilled(
             ImVec2(nextPos.x - 6, nextPos.y - 6),
             ImVec2(nextPos.x - 6, nextPos.y + 6),
             ImVec2(nextPos.x + 4, nextPos.y), Colors::TextSecondary);
         pdl->AddRectFilled(ImVec2(nextPos.x + 6, nextPos.y - 6),
                           ImVec2(nextPos.x + 8, nextPos.y + 6), Colors::TextSecondary);
                           
         ImGui::SetCursorPos(ImVec2(centerX + 35, controlsY)); // 50 - 15 = 35
         if (ImGui::InvisibleButton("##next", ImVec2(30, 30)) && mController) mController->next();
    }
    
    // Progress Bar
    float progY = 60.0f;
    ImGui::SetCursorPos(ImVec2(centerX - 200, progY));
    
    uint32_t elapsedMs = mPlayStartPos;
    if (isPlaying && !mIsDraggingSlider) {
        auto now = std::chrono::steady_clock::now();
        elapsedMs += (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - mPlayStartTime).count();
    }
    uint32_t durationMs = mController && currentTrack >= 0 ? mController->getTrackDuration(currentTrack) * 1000 : 180000;
    if (durationMs == 0) durationMs = 1;

    // Time Text Left
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%d:%02d", (elapsedMs/1000)/60, (elapsedMs/1000)%60);
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    
    // Slider
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 8);
    ImGui::SetNextItemWidth(400);
    float progress = (float)elapsedMs / durationMs;
    if (progress > 1.0f) progress = 1.0f;
    
    // Logic fix: Only seek on release to prevent stutter
    if (ImGui::SliderFloat("##progress", &progress, 0.0f, 1.0f, "")) {
        mIsDraggingSlider = true;
        // Only update UI while dragging, don't seek yet
        mPlayStartPos = (uint32_t)(progress * durationMs);
        mPlayStartTime = std::chrono::steady_clock::now();
    }
    
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        mIsDraggingSlider = false;
        if (mController) mController->seek(mPlayStartPos);
    } else if (ImGui::IsItemDeactivated()) {
        // Fallback if AfterEdit doesn't catch just click
        mIsDraggingSlider = false;
    }
    
    ImGui::PopStyleVar();
    
    ImGui::SameLine();
    
    // Time Text Right
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%d:%02d", (durationMs/1000)/60, (durationMs/1000)%60);
    ImGui::PopStyleColor();
    
    // === RIGHT: Volume ===
    float rightControlsX = mWindowWidth - 150;
    ImGui::SetCursorPos(ImVec2(rightControlsX, 35));
    
    // Volume Icon removed as per request
        
    ImGui::SetCursorPosX(rightControlsX + 25); // Keep spacing or adjust if needed
    int vol = 0;
    if (mController) vol = mController->getVolume();
    float fVol = vol / 100.0f;
    
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderFloat("##vol", &fVol, 0.0f, 1.0f, "")) {
        if (mController) mController->setVolume((int)(fVol * 100));
    }
    
    ImGui::End();
    ImGui::PopStyleColor();
}

} // namespace View
