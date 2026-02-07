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
    
    // Initialize CRUD UI state
    mViewMode = ViewMode::NORMAL;
    mShowDeleteConfirm = false;
    mDeletePlaylistIdx = -1;
    mEditingPlaylistIdx = -1;
    mNewPlaylistName[0] = '\0';
    mNewPlaylistDesc[0] = '\0';
    mNewPlaylistColorIdx = 0;
    mShowAddToPlaylistMenu = false;
    mAddToPlaylistTrackIdx = -1;
    
    // Initialize Session Playlists
    mPlaylists = {
        { "Chill Vibes", "Relaxing acoustic & lofi tracks", 0, {} },
        { "High Energy", "Workout & Upbeat hits", 1, {} },
        { "Focus Flow", "Instrumental study mix", 2, {} }
    };
    
    // Populate Initial Indices (Simulated) - REMOVED as we use paths now and can't easily guess paths
    
    // Initial indices population removed as we now use persistent paths.
    // Real paths will be added by user interaction.
    mDefaultPlaylistsPopulated = false;
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
    
    // Render Modals
    renderCreatePlaylistModal();
    renderEditPlaylistModal();
    renderDeleteConfirmModal();
    renderAddToPlaylistMenu();
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
    ImGui::Text("Music Library (%zu tracks)", mController ? mController->getLibrarySize() : 0);
    ImGui::PopStyleColor();
    
    ImGui::SameLine(mainW - 100);
    if (ImGui::Button("Play All", ImVec2(80, 24)) && mController) {
        // Collect all library tracks and play
        std::vector<std::string> allPaths;
        size_t count = mController->getLibrarySize();
        for (size_t i = 0; i < count; ++i) {
            std::string p = mController->getLibraryTrackPath(i);
            if (!p.empty()) allPaths.push_back(p);
        }
        if (!allPaths.empty()) {
            mController->playPlaylist(allPaths);
        }
    }

    ImGui::Spacing();
    
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    
    size_t libSize = mController ? mController->getLibrarySize() : 0;
    
    for (size_t i = 0; i < libSize; i++) {
        std::string trackName = mController ? mController->getLibraryTrackName(i) : "";
        
        std::string sArtist = mController ? mController->getLibraryTrackArtist(i) : "";
        std::string sAlbum = mController ? mController->getLibraryTrackAlbum(i) : "";
        
        bool match = matchesSearch(trackName, mSearchQuery) || 
                     matchesSearch(sArtist.c_str(), mSearchQuery) || 
                     matchesSearch(sAlbum.c_str(), mSearchQuery);
                     
        if (!match) continue;
        
        // Is this track currently playing?
        // Logic: Check if current playing path matches this library path
        bool isCurrent = false;
        if (currentTrack >= 0 && mController) {
            std::string currPath = mController->getTrackPath(currentTrack); // from queue
            std::string thisPath = mController->getLibraryTrackPath(i); // from library
            if (!currPath.empty() && currPath == thisPath) {
                 isCurrent = true;
            }
        }
        
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
        if (mController) artData = mController->getLibraryTrackCoverArt(i);
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
        ImGui::Text("%s | %s", sArtist.c_str(), sAlbum.c_str());
        ImGui::PopStyleColor();
        
        // Add to playlist button
        ImGui::SameLine(mainW - 60);
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
        if (ImGui::Button("+##add", ImVec2(25, 25))) {
             mShowAddToPlaylistMenu = true;
             mAddToPlaylistTrackIdx = (int)i; // Use library index
        }
        ImGui::PopStyleColor(2);
        
        // Full row clickable -> Play Library Track
        ImGui::SetCursorScreenPos(rowPos);
        if (ImGui::InvisibleButton("##track", ImVec2(mainW - 80, 45)) && mController) {
             std::string p = mController->getLibraryTrackPath(i);
             if (!p.empty()) {
                  // User requested: Keep playlist, insert NEXT, do NOT play immediately (wait for Next button)
                  // Also allows duplicates.
                  mController->queueNext(p);
             }
        }
        
        ImGui::PopID();
    }
    ImGui::Unindent(10);
}


void MainContent::renderPlaylistTab() {
    // Auto-populate default playlists if library is loaded and not yet done
    if (!mDefaultPlaylistsPopulated && mController) {
        size_t totalTracks = mController->getLibrarySize(); // Use Library size
        if (totalTracks > 0) {
            // Distribute tracks: 0-9 to Chill, 10-19 to High Energy, 20-29 to Focus
            for (int i = 0; i < 3 && i < (int)mPlaylists.size(); ++i) {
                int startIdx = i * 10;
                for (int j = 0; j < 10; ++j) {
                    int trackIdx = startIdx + j;
                    if (trackIdx < (int)totalTracks) {
                        std::string path = mController->getLibraryTrackPath(trackIdx); // Use Library accessor
                        if (!path.empty()) {
                            mPlaylists[i].trackPaths.push_back(path);
                        }
                    }
                }
            }
            mDefaultPlaylistsPopulated = true;
        }
    }
    ImGui::Indent(10);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Saved Playlists");
    ImGui::PopStyleColor();
    ImGui::Spacing();
    
    // Use mPlaylists instead of local struct
    
    float btnWidth = 280.0f;
    float btnHeight = 80.0f;
    
    for (int i = 0; i < (int)mPlaylists.size(); i++) {
        ImGui::PushID(i);
        ImVec2 p = ImGui::GetCursorScreenPos();
        
        // Custom Button Background
        ImGui::SetNextItemAllowOverlap(); // Allow clicking buttons rendered on top (must be called before item)
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::HoverV);
        if (ImGui::Button("##plbtn", ImVec2(btnWidth, btnHeight))) {
            // Action: Open Detail View
            mSelectedPlaylistIndex = i;
            mShowPlaylistDetail = true;
        }
        ImGui::PopStyleColor();
        
        // Decoration
        ImDrawList* dl = ImGui::GetWindowDrawList();
        
        // Icon Box (Left side: 10,10 to 70,70 -> 60x60)
        // User Request: Theme is the first song's cover art
        // Icon Box (Left side: 10,10 to 70,70 -> 60x60)
        // User Request: Theme is the first song's cover art
        if (!mPlaylists[i].trackPaths.empty()) {
             // Draw Cover Art
             std::string firstPath = mPlaylists[i].trackPaths[0];
             auto mediaFile = mController ? mController->acquireMediaFile(firstPath) : nullptr;
             std::vector<uint8_t> artData;
             if (mediaFile) artData = mediaFile->getCoverArt();
             
             mAssetManager->drawAlbumCover(dl, ImVec2(p.x + 10, p.y + 10), 60, i * 100, artData); // Use synthetic index for cache key
        } else {
             // Fallback to Color Box if empty
            ImU32 iconCol = IM_COL32(30, 215, 96, 255);
            if (mPlaylists[i].colorIdx == 1) iconCol = IM_COL32(50, 100, 255, 255);
            if (mPlaylists[i].colorIdx == 2) iconCol = IM_COL32(255, 140, 0, 255);
            
            dl->AddRectFilled(ImVec2(p.x + 10, p.y + 10), ImVec2(p.x + 70, p.y + 70), iconCol, 8.0f);
        }
        
        // Text (Name at 80, 15)
        ImGui::SetCursorScreenPos(ImVec2(p.x + 80, p.y + 15));
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
        ImGui::Text("%s", mPlaylists[i].name.c_str());
        ImGui::PopFont();
        
        // Text (Desc at 80, 40)
        ImGui::SetCursorScreenPos(ImVec2(p.x + 80, p.y + 40));
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
        ImGui::Text("%s", mPlaylists[i].desc.c_str());
        ImGui::PopStyleColor();
        
        // Edit/Delete Buttons (Right side)
        // Edit at Width - 95
        ImGui::SetCursorScreenPos(ImVec2(p.x + btnWidth - 95, p.y + 25));
        if (ImGui::SmallButton("Edit")) {
             mViewMode = ViewMode::EDIT_PLAYLIST;
             mEditingPlaylistIdx = i;
             std::strncpy(mNewPlaylistName, mPlaylists[i].name.c_str(), sizeof(mNewPlaylistName)-1);
             std::strncpy(mNewPlaylistDesc, mPlaylists[i].desc.c_str(), sizeof(mNewPlaylistDesc)-1);
             mNewPlaylistColorIdx = mPlaylists[i].colorIdx;
        }
        
        // Del at Width - 45
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        if (ImGui::SmallButton("Del")) {
             mShowDeleteConfirm = true;
             mDeletePlaylistIdx = i;
        }
        ImGui::PopStyleColor();
        
        ImGui::Spacing();
        ImGui::PopID();
        
        // Reset cursor for next item to be below this button
        // Since we messed with cursor pos, let's just make sure we are good.
        // The loop continues and ImGui::Button in next iteration will likely position itself relatively?
        // Actually, ImGui::Button calculates pos based on Layout.
        // We used SetCursorScreenPos which might update the internal cursor?
        // It's safer to ensure we are back in flow.
        // But since we are at end of loop scope, next calling SetCursorPos or having Button auto-layout should work
        // IF we didn't screw up the "Current Window Cursor".
        // SetCursorScreenPos updates the window cursor.
        // So we should reset it to below the button.
        ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + btnHeight + 5)); 
    }

    // Create New Playlist Button
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::HoverV);
    if (ImGui::Button("+ Create New Playlist", ImVec2(btnWidth, 40))) {
        mViewMode = ViewMode::CREATE_PLAYLIST;
        mNewPlaylistName[0] = '\0';
        mNewPlaylistDesc[0] = '\0';
        mNewPlaylistColorIdx = 0;
    }
    ImGui::PopStyleColor();
    
    ImGui::Unindent(10);
}

void MainContent::renderPlaylistDetailView() {
    // Use mPlaylists

    if (mSelectedPlaylistIndex < 0 || mSelectedPlaylistIndex >= (int)mPlaylists.size()) {
        mShowPlaylistDetail = false;
        return;
    }
    
    const auto& pl = mPlaylists[mSelectedPlaylistIndex];

    // Header with Back Button
    ImGui::Indent(10);
    if (ImGui::Button("<- Back")) {
        mShowPlaylistDetail = false;
        mSelectedPlaylistIndex = -1;
    }
    ImGui::SameLine();
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%s", pl.name.c_str());
    ImGui::PopFont();
    
    ImGui::Spacing();
    
    // Play All Button
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::GreenV);
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::BlackV);
    if (ImGui::Button("PLAY ALL", ImVec2(120, 36))) {
        if (mController && !pl.trackPaths.empty()) {
             // Logic: If queue has items, append. If empty, replace and play.
             if (mController->getPlaylistSize() > 0) {
                 mController->queuePlaylist(pl.trackPaths);
             } else {
                 mController->playPlaylist(pl.trackPaths);
             }
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
    
    
    // Iterate paths
    for (size_t i = 0; i < pl.trackPaths.size(); ++i) {
        std::string path = pl.trackPaths[i];
        auto mediaFile = mController ? mController->acquireMediaFile(path) : nullptr;
        
        ImGui::PushID((int)i);
        ImVec2 rowPos = ImGui::GetCursorScreenPos();
        
        // Album Cover
        std::vector<uint8_t> artData;
        if (mediaFile) artData = mediaFile->getCoverArt();
        
        mAssetManager->drawAlbumCover(cdl, ImVec2(rowPos.x + 10, rowPos.y), 40, (int)i + 1000 + mSelectedPlaylistIndex*100, artData);
        
        // Text
        std::string tName = mediaFile ? mediaFile->getFilename() : "Unknown";
        std::string tArtist = mediaFile ? mediaFile->getArtist() : "Unknown";
        std::string tAlbum = mediaFile ? mediaFile->getAlbum() : "Unknown";
        
        if (tName.empty()) tName = "Track";
        
        std::string dispName = stripExtension(tName);
        if (dispName.length() > 35) dispName = dispName.substr(0, 32) + "...";
        
        ImGui::SetCursorPosX(60);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("%s", dispName.c_str());
        ImGui::PopStyleColor();
        
        ImGui::SetCursorPosX(60);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
        ImGui::Text("%s | %s", tArtist.c_str(), tAlbum.c_str());
        ImGui::PopStyleColor();
        
        // Delete Button
        ImGui::SetCursorPosX(mainW - 50);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
        if (ImGui::Button("X", ImVec2(30, 30))) {
            // Remove from vector
            mPlaylists[mSelectedPlaylistIndex].trackPaths.erase(mPlaylists[mSelectedPlaylistIndex].trackPaths.begin() + i);
            ImGui::PopStyleColor();
            ImGui::PopID();
            break; // Break loop
        }
        ImGui::PopStyleColor();
        
        // Click to Play specific track in context
        ImGui::SetCursorScreenPos(rowPos);
        if (ImGui::InvisibleButton("##pldet", ImVec2(mainW - 60, 45)) && mController) {
             // Logic: If playing, append whole playlist but jump to this track? 
             // Or just replace queue starting with this track?
             // User just asked for "play playlist" behavior.
             // For specific track click, usually users expect immediate play.
             // Let's keep it safe: Replace queue.
             mController->playPlaylist(pl.trackPaths);
             mController->playTrack((int)i);
        }
        
        ImGui::PopID();
    }
    
    ImGui::Unindent(10);

}

void MainContent::createPlaylist() {
    if (std::strlen(mNewPlaylistName) == 0) return;
    
    PlaylistData pd;
    pd.name = mNewPlaylistName;
    pd.desc = mNewPlaylistDesc;
    pd.colorIdx = mNewPlaylistColorIdx;
    
    mPlaylists.push_back(pd);
    mViewMode = ViewMode::NORMAL;
}

void MainContent::updatePlaylist() {
    if (mEditingPlaylistIdx < 0 || mEditingPlaylistIdx >= (int)mPlaylists.size()) return;
    if (std::strlen(mNewPlaylistName) == 0) return;
    
    mPlaylists[mEditingPlaylistIdx].name = mNewPlaylistName;
    mPlaylists[mEditingPlaylistIdx].desc = mNewPlaylistDesc;
    mPlaylists[mEditingPlaylistIdx].colorIdx = mNewPlaylistColorIdx;
    
    mViewMode = ViewMode::NORMAL;
    mEditingPlaylistIdx = -1;
}

void MainContent::deletePlaylist(int idx) {
    if (idx >= 0 && idx < (int)mPlaylists.size()) {
        mPlaylists.erase(mPlaylists.begin() + idx);
        if (mSelectedPlaylistIndex == idx) {
            mShowPlaylistDetail = false;
            mSelectedPlaylistIndex = -1;
        } else if (mSelectedPlaylistIndex > idx) {
            mSelectedPlaylistIndex--;
        }
    }
    mShowDeleteConfirm = false;
    mDeletePlaylistIdx = -1;
}

void MainContent::addTrackToPlaylist(int trackIdx, int playlistIdx) {
    if (playlistIdx >= 0 && playlistIdx < (int)mPlaylists.size() && mController) {
        // Fix: Use getLibraryTrackPath because the index comes from the Music Tab (Library)
        std::string path = mController->getLibraryTrackPath(trackIdx);
        if (!path.empty()) {
            mPlaylists[playlistIdx].trackPaths.push_back(path);
        }
    }
}

void MainContent::renderCreatePlaylistModal() {
    if (mViewMode == ViewMode::CREATE_PLAYLIST) {
        ImGui::OpenPopup("Create Playlist");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 160)); // Compact height

    // Use NoTitleBar for custom header
    if (ImGui::BeginPopupModal("Create Playlist", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        
        // Custom Header
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
        ImGui::Text("Create Playlist");
        ImGui::PopFont();
        
        // Close Button (X)
        ImGui::SameLine(ImGui::GetWindowWidth() - 35);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0));
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
        if (ImGui::Button("X", ImVec2(25, 25))) {
             mViewMode = ViewMode::NORMAL;
             ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);
        
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Content
        ImGui::Indent(20);
        
        ImGui::Text("Name");
        ImGui::PushItemWidth(320);
        ImGui::InputText("##Name", mNewPlaylistName, 128);
        ImGui::PopItemWidth();
        
        // Removed Description & Color Theme as requested
        
        ImGui::Unindent(20);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Create Button (Centered)
        float btnW = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - btnW) * 0.5f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::GreenV);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::BlackV);
        if (ImGui::Button("Create", ImVec2(btnW, 35))) {
            createPlaylist();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);
        
        ImGui::EndPopup();
    }
}

void MainContent::renderEditPlaylistModal() {
    if (mViewMode == ViewMode::EDIT_PLAYLIST) {
        ImGui::OpenPopup("Edit Playlist");
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 160)); // Compact height

    // Use NoTitleBar to implement custom header with X button
    if (ImGui::BeginPopupModal("Edit Playlist", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {
        
        // Custom Header: Title Left, X Right
        ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); 
        ImGui::Text("Edit Playlist");
        ImGui::PopFont();
        
        ImGui::SameLine(ImGui::GetWindowWidth() - 35);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0,0,0,0)); // Transparent background
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
        if (ImGui::Button("X", ImVec2(25, 25))) {
             mViewMode = ViewMode::NORMAL;
             mEditingPlaylistIdx = -1;
             ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);
        
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::Spacing();
        
        // Indented Content
        ImGui::Indent(20);
        
        ImGui::Text("Name");
        ImGui::PushItemWidth(320);
        ImGui::InputText("##Name", mNewPlaylistName, 128);
        ImGui::PopItemWidth();
        
        ImGui::Unindent(20);
        
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        // Save Button (Centered)
        float saveBtnW = 120.0f;
        ImGui::SetCursorPosX((ImGui::GetWindowWidth() - saveBtnW) * 0.5f);
        
        ImGui::PushStyleColor(ImGuiCol_Button, Colors::GreenV);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::BlackV);
        if (ImGui::Button("Save", ImVec2(saveBtnW, 35))) {
            updatePlaylist();
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(2);
        
        ImGui::EndPopup();
    }
}

void MainContent::renderDeleteConfirmModal() {
    if (mShowDeleteConfirm) {
        ImGui::OpenPopup("Delete Playlist?");
        mShowDeleteConfirm = false; // Reset trigger immediately
    }

    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Delete Playlist?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this playlist?");
        ImGui::Text("This action cannot be undone.");
        
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            deletePlaylist(mDeletePlaylistIdx);
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor();
        
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            mDeletePlaylistIdx = -1; // Clear selection
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}

void MainContent::renderAddToPlaylistMenu() {
    if (mShowAddToPlaylistMenu) {
        ImGui::OpenPopup("Add to Playlist");
        mShowAddToPlaylistMenu = false; // Reset trigger immediately
    }
    
    if (ImGui::BeginPopup("Add to Playlist")) {
        ImGui::Text("Select Playlist:");
        ImGui::Separator();
        
        for (int i = 0; i < (int)mPlaylists.size(); i++) {
            if (ImGui::Selectable(mPlaylists[i].name.c_str())) {
                addTrackToPlaylist(mAddToPlaylistTrackIdx, i);
            }
        }
        
        if (ImGui::Selectable("+ Create New Playlist")) {
            mViewMode = ViewMode::CREATE_PLAYLIST;
            // Pre-fill creation modal? Maybe later.
            // For now just open creation mode
            mNewPlaylistName[0] = '\0';
            mNewPlaylistDesc[0] = '\0';
            mNewPlaylistColorIdx = 0;
        }
        
        ImGui::EndPopup();
    }
}

} // namespace View
