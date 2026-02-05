/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/MainContent.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of MainContent.
 */

#include "MainContent.h"
#include "../../imgui/imgui.h"
#include <algorithm>
#include <cstring>

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
    const ImVec4 TransparentV = ImVec4(0, 0, 0, 0);
}

MainContent::MainContent(std::shared_ptr<Controller::IAppController> controller,
                         std::shared_ptr<Model::IPlayerState> playerState,
                         std::shared_ptr<AssetManager> assetManager)
    : mController(controller)
    , mPlayerState(playerState)
    , mAssetManager(assetManager)
    , mMainTabIndex(1)
{
    mSearchQuery[0] = '\0';
    mShowPlaylistDetail = false;
    mSelectedPlaylistIndex = -1;
}

void MainContent::setPlaylist(const std::vector<std::string>& playlist) {
    mPlaylistDisplay = playlist;
}

std::string MainContent::stripExtension(const std::string& name) {
    if (name.length() > 4 && name.substr(name.length()-4) == ".mp3")
        return name.substr(0, name.length() - 4);
    if (name.length() > 4 && name.substr(name.length()-4) == ".wav")
        return name.substr(0, name.length() - 4);
    return name;
}

bool MainContent::matchesSearch(const std::string& text, const char* query) {
    if (query[0] == '\0') return true;
    std::string lowerText = text;
    std::string lowerQuery = query;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    return lowerText.find(lowerQuery) != std::string::npos;
}

void MainContent::render() {
    int windowWidth = ImGui::GetIO().DisplaySize.x;
    int windowHeight = ImGui::GetIO().DisplaySize.y;
    
    // Layout
    const float rightSidebarW = 300.0f;
    const float playerBarH = 90.0f;
    const float gap = 8.0f;
    const float mainW = windowWidth - rightSidebarW - gap * 2;
    const float contentH = windowHeight - playerBarH - gap; // Reduced height to not overlap player

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

    ImGui::SetNextWindowPos(ImVec2(gap, gap));
    ImGui::SetNextWindowSize(ImVec2(mainW, contentH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::SurfaceV);
    
    ImGui::Begin("MainContent", nullptr, flags);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wp = ImGui::GetWindowPos();
    
    // Background
    dl->AddRectFilled(wp, ImVec2(wp.x + mainW, wp.y + contentH), Colors::Surface, 8.0f);
    
    renderSearch();
    renderTabs();
    
    // ===== TAB CONTENT =====
    ImGui::SetCursorPos(ImVec2(10, 60));
    ImGui::BeginChild("TabContent", ImVec2(mainW - 20, contentH - 70), false);
    
    if (mMainTabIndex == 0) renderRecentTab(mainW, contentH);
    else if (mMainTabIndex == 1) renderMusicTab(mainW, contentH);
    else if (mMainTabIndex == 2) {
        if (mShowPlaylistDetail) renderPlaylistDetailView();
        else renderPlaylistTab();
    }
    
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();
}

void MainContent::renderSearch() {
    ImGui::SetCursorPos(ImVec2(20, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::HoverV);
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##search", "Search songs, artists...", mSearchQuery, 256);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

void MainContent::renderTabs() {
    ImGui::SetCursorPos(ImVec2(340, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    
    const char* tabs[] = {"All", "Music", "Playlists"};
    for (int i = 0; i < 3; i++) {
        if (i > 0) ImGui::SameLine();
        
        bool selected = (mMainTabIndex == i);
        ImGui::PushStyleColor(ImGuiCol_Button, selected ? Colors::WhiteV : Colors::HoverV);
        ImGui::PushStyleColor(ImGuiCol_Text, selected ? Colors::BlackV : Colors::WhiteV);
        
        if (ImGui::Button(tabs[i], ImVec2(80, 28))) mMainTabIndex = i;
        ImGui::PopStyleColor(2);
    }
    ImGui::PopStyleVar();
}

void MainContent::renderRecentTab(float mainW, float contentH) {
    (void)contentH;
    ImGui::Indent(10);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Recently Played");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    std::vector<int> history;
    if (mController) history = mController->getHistory();
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    
    if (history.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
        ImGui::Text("No recently played tracks.");
        ImGui::PopStyleColor();
    } else {
        ImDrawList* cdl = ImGui::GetWindowDrawList();
        
        for (int i = (int)history.size() - 1; i >= 0; i--) {
            int trackIdx = history[i];
            if (trackIdx < 0) continue;
            
            std::string tName = mController->getTrackName(trackIdx);
            std::string tArtist = mController->getTrackArtist(trackIdx);
            
            bool isCurrent = (currentTrack == trackIdx);
            
            if (!matchesSearch(tName, mSearchQuery) && !matchesSearch(tArtist.c_str(), mSearchQuery)) continue;
            
            ImGui::PushID((int)(2000 + i));
            ImVec2 rowPos = ImGui::GetCursorScreenPos();
            
            // Album cover - Get data
            std::vector<uint8_t> artData;
            if (mController) artData = mController->getTrackCoverArt(trackIdx);
            mAssetManager->drawAlbumCover(cdl, rowPos, 40, trackIdx, artData);
            
            // Track info
            std::string dispName = stripExtension(tName);
            if (dispName.length() > 35) dispName = dispName.substr(0, 32) + "...";
            
            ImGui::SetCursorPosX(60);
            ImGui::PushStyleColor(ImGuiCol_Text, isCurrent ? Colors::GreenV : Colors::WhiteV);
            ImGui::Text("%s", dispName.c_str());
            ImGui::PopStyleColor();
            
            ImGui::SetCursorPosX(60);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
            std::string tAlbum = mController->getTrackAlbum(trackIdx);
            ImGui::Text("%s | %s", tArtist.c_str(), tAlbum.c_str());
            ImGui::PopStyleColor();
            
            // Click to play
            ImGui::SetCursorScreenPos(rowPos);
            if (ImGui::InvisibleButton("##recent", ImVec2(mainW - 80, 45)) && mController) {
                 mController->playTrack(trackIdx);
            }
            ImGui::PopID();
        }
    }
    ImGui::Unindent(10);
}

void MainContent::renderMusicTab(float mainW, float contentH) {
    (void)contentH;
    ImGui::Indent(10);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Music Library (%zu tracks)", mPlaylistDisplay.size());
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    
    for (size_t i = 0; i < mPlaylistDisplay.size(); i++) {
        std::string trackName = mPlaylistDisplay[i];
        
        std::string sArtist = mController ? mController->getTrackArtist(i) : "";
        std::string sAlbum = mController ? mController->getTrackAlbum(i) : "";
        
        bool match = matchesSearch(trackName, mSearchQuery) || 
                     matchesSearch(sArtist.c_str(), mSearchQuery) || 
                     matchesSearch(sAlbum.c_str(), mSearchQuery);
                     
        if (!match) continue;
        
        bool isCurrent = (currentTrack == (int)i);
        
        ImGui::PushID((int)i);
        ImVec2 rowPos = ImGui::GetCursorScreenPos();
        
        // Track number
        ImGui::PushStyleColor(ImGuiCol_Text, isCurrent ? Colors::GreenV : Colors::TextMutedV);
        ImGui::Text("%s", isCurrent ? " *" : "");
        ImGui::SameLine();
        if (!isCurrent) ImGui::Text("%2zu", i + 1);
        ImGui::PopStyleColor();
        
        ImGui::SameLine();
        
        // Album cover
        std::vector<uint8_t> artData;
        if (mController) artData = mController->getTrackCoverArt(i);
        mAssetManager->drawAlbumCover(cdl, ImVec2(rowPos.x + 40, rowPos.y), 40, (int)i, artData);
        
        // Track info
        std::string dispName = stripExtension(trackName);
        if (dispName.length() > 45) dispName = dispName.substr(0, 42) + "...";
        
        ImGui::SetCursorPosX(95);
        ImGui::PushStyleColor(ImGuiCol_Text, isCurrent ? Colors::GreenV : Colors::WhiteV);
        ImGui::Text("%s", dispName.c_str());
        ImGui::PopStyleColor();
        
        ImGui::SetCursorPosX(95);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
        std::string artistStr = mController ? mController->getTrackArtist(i) : "Unknown Artist";
        std::string albumStr = mController ? mController->getTrackAlbum(i) : "Unknown Album";
        ImGui::Text("%s | %s", artistStr.c_str(), albumStr.c_str());
        ImGui::PopStyleColor();
        
        // Add to playlist button
        ImGui::SameLine(mainW - 60);
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
        if (ImGui::Button("+##add", ImVec2(25, 25))) {
            // TODO
        }
        ImGui::PopStyleColor(2);
        
        // Full row clickable
        ImGui::SetCursorScreenPos(rowPos);
        if (ImGui::InvisibleButton("##track", ImVec2(mainW - 80, 45)) && mController) {
            mController->playTrack((int)i);
        }
        
        ImGui::PopID();
    }
    ImGui::Unindent(10);
}


void MainContent::renderPlaylistTab() {
    ImGui::Indent(10);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Saved Playlists");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Predefined Playlists Data
    struct DemoPlaylist {
        const char* name;
        const char* desc;
        int colorIdx; // 0=Green, 1=Blue, 2=Orange
        int startTrack;
        int count;
    };
    
    DemoPlaylist demos[] = {
        { "Chill Vibes", "Relaxing acoustic & lofi tracks", 0, 0, 10 },
        { "High Energy", "Workout & Upbeat hits", 1, 10, 10 },
        { "Focus Flow", "Instrumental study mix", 2, 20, 10 }
    };
    
    float btnWidth = 280.0f;
    float btnHeight = 80.0f;
    
    for (int i = 0; i < 3; i++) {
        ImGui::PushID(i);
        ImVec2 p = ImGui::GetCursorScreenPos();
        
        // Custom Button Background
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::HoverV);
        if (ImGui::Button("##plbtn", ImVec2(btnWidth, btnHeight))) {
            // Action: Open Detail View
            mSelectedPlaylistIndex = i;
            mShowPlaylistDetail = true;
        }
        ImGui::PopStyleColor();
        
        // Decoration
        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        // Icon Box
        ImU32 iconCol = IM_COL32(30, 215, 96, 255);
        if (demos[i].colorIdx == 1) iconCol = IM_COL32(50, 100, 255, 255);
        if (demos[i].colorIdx == 2) iconCol = IM_COL32(255, 140, 0, 255);
        
        dl->AddRectFilled(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 70, p.y + 70), iconCol, 8.0f);
        
        // Text
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 80, ImGui::GetCursorPosY() - btnHeight + 15));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
        ImGui::Text("%s", demos[i].name);
        ImGui::PopFont();
        
        ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + 80, ImGui::GetCursorPosY() + 5));
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
        ImGui::Text("%s", demos[i].desc);
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::PopID();
    }
    
    ImGui::Unindent(10);
}

void MainContent::renderPlaylistDetailView() {
    // Shared Data Definition (duplicated locally for simplicity)
    struct DemoPlaylist {
        const char* name;
        const char* desc;
        int colorIdx;
        int startTrack;
        int count;
    };
    DemoPlaylist demos[] = {
        { "Chill Vibes", "Relaxing acoustic & lofi tracks", 0, 0, 10 },
        { "High Energy", "Workout & Upbeat hits", 1, 10, 10 },
        { "Focus Flow", "Instrumental study mix", 2, 20, 10 }
    };

    if (mSelectedPlaylistIndex < 0 || mSelectedPlaylistIndex >= 3) {
        mShowPlaylistDetail = false;
        return;
    }
    
    DemoPlaylist& pl = demos[mSelectedPlaylistIndex];

    // Header with Back Button
    ImGui::Indent(10);
    if (ImGui::Button("<- Back")) {
        mShowPlaylistDetail = false;
        mSelectedPlaylistIndex = -1;
    }
    ImGui::SameLine();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%s", pl.name);
    ImGui::PopFont();
    
    ImGui::Spacing();
    
    // Play All Button
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::GreenV);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::BlackV);
    if (ImGui::Button("PLAY ALL", ImVec2(120, 36))) {
        if (mController) {
             mController->clearPlaylist();
             size_t libSize = mController->loadDirectory("./mMusic"); 
             int start = pl.startTrack;
             int end = start + pl.count;
             if (end > (int)libSize) end = (int)libSize;
             
             // Just play from start logic
             mController->playTrack(start);
        }
    }
    ImGui::PopStyleColor(2);
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Track List
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    float mainW = ImGui::GetWindowWidth();
    
    // Safety check for library size
    // We assume library is loaded or load it now
    if (mController) mController->loadDirectory("./mMusic");
    
    int start = pl.startTrack;
    int end = start + pl.count;
    // Bounds check? Logic assumed library is big enough for demo
    
    for (int i = start; i < end; ++i) {
        ImGui::PushID(i);
        ImVec2 rowPos = ImGui::GetCursorScreenPos();
        
        // Album Cover
        std::vector<uint8_t> artData;
        if (mController) artData = mController->getTrackCoverArt(i);
        // Note: Check bounds in controller happens inside getTrackCoverArt usually
        mAssetManager->drawAlbumCover(cdl, ImVec2(rowPos.x + 10, rowPos.y), 40, i, artData);
        
        // Text
        std::string tName = mController ? mController->getTrackName(i) : "";
        std::string tArtist = mController ? mController->getTrackArtist(i) : "";
        std::string tAlbum = mController ? mController->getTrackAlbum(i) : "";
        
        if (tName.empty()) tName = "Track " + std::to_string(i);
        
        std::string dispName = stripExtension(tName);
        if (dispName.length() > 40) dispName = dispName.substr(0, 37) + "...";
        
        ImGui::SetCursorPosX(60);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("%s", dispName.c_str());
        ImGui::PopStyleColor();
        
        ImGui::SetCursorPosX(60);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
        ImGui::Text("%s | %s", tArtist.c_str(), tAlbum.c_str());
        ImGui::PopStyleColor();
        
        // Click to Play specific track
        ImGui::SetCursorScreenPos(rowPos);
        if (ImGui::InvisibleButton("##pldet", ImVec2(mainW - 40, 45)) && mController) {
             mController->playTrack(i);
        }
        
        ImGui::PopID();
    }
    
    ImGui::Unindent(10);

}

} // namespace View
