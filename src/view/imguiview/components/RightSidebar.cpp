/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/RightSidebar.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of RightSidebar.
 */

#include "RightSidebar.h"
#include "../../imgui/imgui.h"

namespace View {

// Minimal Color Palette Reuse
namespace Colors {
    const ImU32 Black       = IM_COL32(0, 0, 0, 255);
    const ImU32 Surface     = IM_COL32(18, 18, 18, 255);
    const ImVec4 SurfaceV   = ImVec4(0.07f, 0.07f, 0.07f, 1);
    const ImVec4 HoverV     = ImVec4(0.24f, 0.24f, 0.24f, 1);
    const ImVec4 GreenV     = ImVec4(0.12f, 0.84f, 0.38f, 1);
    const ImVec4 WhiteV     = ImVec4(1, 1, 1, 1);
    const ImVec4 BlackV     = ImVec4(0, 0, 0, 1);
    const ImVec4 TextSecV   = ImVec4(0.70f, 0.70f, 0.70f, 1);
    const ImVec4 TextMutedV = ImVec4(0.45f, 0.45f, 0.45f, 1);
    
    // For DrawList
    const ImU32 Green       = IM_COL32(30, 215, 96, 255);
    const ImU32 White       = IM_COL32(255, 255, 255, 255);
    const ImU32 TextSecondary = IM_COL32(179, 179, 179, 255);
    const ImU32 TextMuted   = IM_COL32(115, 115, 115, 255);
}

RightSidebar::RightSidebar(std::shared_ptr<Controller::IAppController> controller,
                           std::shared_ptr<Model::IPlayerState> playerState,
                           std::shared_ptr<AssetManager> assetManager)
    : mController(controller)
    , mPlayerState(playerState)
    , mAssetManager(assetManager)
    , mRightTabIndex(0)
{}

void RightSidebar::setPlaylist(const std::vector<std::string>& playlist) {
    mPlaylistDisplay = playlist;
}

std::string RightSidebar::stripExtension(const std::string& name) {
    if (name.length() > 4 && name.substr(name.length()-4) == ".mp3")
        return name.substr(0, name.length() - 4);
    if (name.length() > 4 && name.substr(name.length()-4) == ".wav")
        return name.substr(0, name.length() - 4);
    return name;
}

void RightSidebar::render() {
    int windowWidth = ImGui::GetIO().DisplaySize.x;
    int windowHeight = ImGui::GetIO().DisplaySize.y;
    
    // Layout
    const float rightSidebarW = 300.0f;
    const float playerBarH = 90.0f;
    const float gap = 8.0f;
    const float contentH = windowHeight - playerBarH - gap;

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

    ImGui::SetNextWindowPos(ImVec2(windowWidth - rightSidebarW - gap, gap));
    ImGui::SetNextWindowSize(ImVec2(rightSidebarW, contentH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::SurfaceV);
    
    ImGui::Begin("RightSidebar", nullptr, flags);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wp = ImGui::GetWindowPos();
    
    // Background
    dl->AddRectFilled(wp, ImVec2(wp.x + rightSidebarW, wp.y + contentH), Colors::Surface, 8.0f);
    
    renderTabs();
    renderNowPlaying(dl);
    
    // Queue/Recent List Header
    ImGui::SetCursorPos(ImVec2(15, 160));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text(mRightTabIndex == 0 ? "Next up" : "Recently played");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(10, 185));
    ImGui::BeginChild("QueueList", ImVec2(rightSidebarW - 20, contentH - 205), false);
    
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    
    if (mRightTabIndex == 0) renderQueue(dl, rightSidebarW, currentTrack);
    else renderRecent(dl, rightSidebarW);
    
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();
}

void RightSidebar::renderTabs() {
    ImGui::SetCursorPos(ImVec2(15, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button, mRightTabIndex == 0 ? Colors::GreenV : Colors::HoverV);
    ImGui::PushStyleColor(ImGuiCol_Text, mRightTabIndex == 0 ? Colors::BlackV : Colors::WhiteV);
    if (ImGui::Button("Queue", ImVec2(80, 26))) mRightTabIndex = 0;
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button, mRightTabIndex == 1 ? Colors::GreenV : Colors::HoverV);
    ImGui::PushStyleColor(ImGuiCol_Text, mRightTabIndex == 1 ? Colors::BlackV : Colors::WhiteV);
    if (ImGui::Button("Recent", ImVec2(80, 26))) mRightTabIndex = 1;
    ImGui::PopStyleColor(2);
    
    ImGui::PopStyleVar();
}

void RightSidebar::renderNowPlaying(ImDrawList* dl) {
    ImGui::SetCursorPos(ImVec2(15, 55));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Now playing");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(15, 80));
    ImVec2 npPos = ImGui::GetCursorScreenPos();
    
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    std::vector<uint8_t> artData;
    if (mController && currentTrack >= 0) artData = mController->getTrackCoverArt(currentTrack);
    
    mAssetManager->drawAlbumCover(dl, npPos, 65, currentTrack >= 0 ? currentTrack : 0, artData);
    
    std::string npName = "No Track Selected";
    if (currentTrack >= 0 && currentTrack < (int)mPlaylistDisplay.size()) {
        npName = stripExtension(mPlaylistDisplay[currentTrack]);
    }
    if (npName.length() > 22) npName = npName.substr(0, 19) + "...";
    
    dl->AddText(ImVec2(npPos.x + 75, npPos.y + 10), Colors::Green, npName.c_str());
    std::string npArtist = mController && currentTrack >= 0 ? mController->getTrackArtist(currentTrack) : "Unknown Artist";
    std::string npAlbum = mController && currentTrack >= 0 ? mController->getTrackAlbum(currentTrack) : "Unknown Album";
    dl->AddText(ImVec2(npPos.x + 75, npPos.y + 30), Colors::TextSecondary, npArtist.c_str());
    dl->AddText(ImVec2(npPos.x + 75, npPos.y + 46), Colors::TextMuted, npAlbum.c_str());
}

void RightSidebar::renderQueue(ImDrawList* dl, float width, int currentTrack) {
    if (currentTrack < 0) return;
    
    for (int i = currentTrack + 1; i < (int)mPlaylistDisplay.size() && i < currentTrack + 10; i++) {
        ImGui::PushID(4000 + i);
        ImVec2 tPos = ImGui::GetCursorScreenPos();
        
        std::vector<uint8_t> artData;
        if (mController) artData = mController->getTrackCoverArt(i);
        mAssetManager->drawAlbumCover(dl, tPos, 40, i, artData);
        
        std::string tName = stripExtension(mPlaylistDisplay[i]);
        if (tName.length() > 20) tName = tName.substr(0, 17) + "...";
        
        dl->AddText(ImVec2(tPos.x + 50, tPos.y + 5), Colors::White, tName.c_str());
        std::string tArtist = mController ? mController->getTrackArtist(i) : "Unknown Artist";
        dl->AddText(ImVec2(tPos.x + 50, tPos.y + 22), Colors::TextSecondary, tArtist.c_str());
        
        ImGui::InvisibleButton("##q", ImVec2(width - 40, 45));
        if (ImGui::IsItemClicked() && mController) {
            mController->playTrack(i);
        }
        ImGui::PopID();
    }
}

void RightSidebar::renderRecent(ImDrawList* dl, float width) {
    if (mController) {
        size_t historyCount = mController->getHistorySize();
        
        // Show last 10 items (or all if < 10)
        // History Stack: [Oldest ... Newest]
        // We want to show Newest at top.
        // So iterate from size-1 down to max(0, size-10)
        
        int count = 0;
        for (int i = (int)historyCount - 1; i >= 0 && count < 10; i--, count++) {
            ImGui::PushID(5000 + i);
            ImVec2 tPos = ImGui::GetCursorScreenPos();
            
            std::vector<uint8_t> artData = mController->getHistoryTrackCoverArt(i);
            mAssetManager->drawAlbumCover(dl, tPos, 40, i + 5000, artData); // Use synthetic index + offset for cache
            
            std::string tName = mController->getHistoryTrackName(i);
            tName = stripExtension(tName);
            if (tName.length() > 20) tName = tName.substr(0, 17) + "...";
            
            dl->AddText(ImVec2(tPos.x + 50, tPos.y + 5), Colors::White, tName.c_str());

            std::string tArtist = mController->getHistoryTrackArtist(i);
            if (tArtist.length() > 25) tArtist = tArtist.substr(0, 22) + "...";
            dl->AddText(ImVec2(tPos.x + 50, tPos.y + 22), Colors::TextSecondary, tArtist.c_str());
            
            ImGui::InvisibleButton("##rside", ImVec2(width - 40, 45));
            if (ImGui::IsItemClicked()) {
                 mController->playHistoryTrack(i);
            }
            ImGui::PopID();
            ImGui::Dummy(ImVec2(0, 5));
        }
    }
}

} // namespace View
