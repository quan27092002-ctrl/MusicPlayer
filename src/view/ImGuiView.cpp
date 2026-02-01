/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.cpp
 * DESCRIPTION: Spotify-style UI with all features.
 */

#include "ImGuiView.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include <SDL2/SDL_image.h>
#include <cstdio>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <map>

namespace View {

// Spotify Color Palette
namespace Colors {
    const ImU32 Black       = IM_COL32(0, 0, 0, 255);
    const ImU32 Surface     = IM_COL32(18, 18, 18, 255);
    const ImU32 SurfaceLight= IM_COL32(40, 40, 40, 255);
    const ImU32 Hover       = IM_COL32(60, 60, 60, 255);
    const ImU32 Green       = IM_COL32(30, 215, 96, 255);
    const ImU32 White       = IM_COL32(255, 255, 255, 255);
    const ImU32 TextSecondary = IM_COL32(179, 179, 179, 255);
    const ImU32 TextMuted   = IM_COL32(115, 115, 115, 255);
    
    const ImVec4 BlackV     = ImVec4(0, 0, 0, 1);
    const ImVec4 SurfaceV   = ImVec4(0.07f, 0.07f, 0.07f, 1);
    const ImVec4 SurfaceLightV = ImVec4(0.16f, 0.16f, 0.16f, 1);
    const ImVec4 HoverV     = ImVec4(0.24f, 0.24f, 0.24f, 1);
    const ImVec4 GreenV     = ImVec4(0.12f, 0.84f, 0.38f, 1);
    const ImVec4 WhiteV     = ImVec4(1, 1, 1, 1);
    const ImVec4 TextSecV   = ImVec4(0.70f, 0.70f, 0.70f, 1);
    const ImVec4 TextMutedV = ImVec4(0.45f, 0.45f, 0.45f, 1);
    const ImVec4 TransparentV = ImVec4(0, 0, 0, 0);
}

// Structs moved to header
// Globals moved to class members
// Structs moved to header
// Globals moved to class members in header

// ============================================================================
// Constructor / Destructor
// ============================================================================

ImGuiView::ImGuiView(
    std::shared_ptr<Controller::IAppController> controller,
    std::shared_ptr<Model::IPlayerState> playerState
)
    : mController(controller)
    , mPlayerState(playerState)
    , mWindow(nullptr)
    , mRenderer(nullptr)
    , mRunning(false)
    , mWindowWidth(1080)
    , mWindowHeight(720) {
        // Initialize state
        mMainTabIndex = 1;
        mRightTabIndex = 0;
        mPlayStartPos = 0;
        mWasPlaying = false;
        mSearchQuery[0] = '\0';
    }

ImGuiView::~ImGuiView() {
    shutdown();
}

// ============================================================================
// Theme Setup
// ============================================================================

void setupSpotifyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    style.WindowRounding = 0.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 4.0f;
    style.PopupRounding = 6.0f;
    style.ScrollbarRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    
    style.WindowPadding = ImVec2(0, 0);
    style.FramePadding = ImVec2(8, 4);
    style.ItemSpacing = ImVec2(8, 4);
    style.ScrollbarSize = 8.0f;
    style.GrabMinSize = 8.0f;
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 0.0f;
    
    ImVec4* c = style.Colors;
    c[ImGuiCol_WindowBg] = Colors::BlackV;
    c[ImGuiCol_ChildBg] = Colors::TransparentV;
    c[ImGuiCol_FrameBg] = Colors::SurfaceLightV;
    c[ImGuiCol_FrameBgHovered] = Colors::HoverV;
    c[ImGuiCol_FrameBgActive] = Colors::HoverV;
    c[ImGuiCol_ScrollbarBg] = Colors::TransparentV;
    c[ImGuiCol_ScrollbarGrab] = Colors::HoverV;
    c[ImGuiCol_SliderGrab] = Colors::WhiteV;
    c[ImGuiCol_SliderGrabActive] = Colors::WhiteV;
    c[ImGuiCol_Button] = Colors::SurfaceLightV;
    c[ImGuiCol_ButtonHovered] = Colors::HoverV;
    c[ImGuiCol_ButtonActive] = Colors::HoverV; // Was GreenV
    c[ImGuiCol_Header] = Colors::SurfaceLightV;
    c[ImGuiCol_HeaderHovered] = Colors::HoverV;
    c[ImGuiCol_Text] = Colors::WhiteV;
    c[ImGuiCol_TextDisabled] = Colors::TextMutedV;
}

// ============================================================================
// IView Interface
// ============================================================================

bool ImGuiView::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    mWindow = SDL_CreateWindow(
        "",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        mWindowWidth, mWindowHeight,
        SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!mWindow) return false;

    mRenderer = SDL_CreateRenderer(mWindow, -1, 
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (!mRenderer) {
        SDL_DestroyWindow(mWindow);
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
    ImGui_ImplSDLRenderer2_Init(mRenderer);
    
    // Load better font
    io.Fonts->Clear();
    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf", 16.0f);
    if (!font) {
        io.Fonts->AddFontDefault();
    }

    setupSpotifyTheme();
    mRunning = true;
    return true;
}

void ImGuiView::shutdown() {
    std::cerr << "ImGuiView::shutdown() called." << std::endl;
    if (!mWindow) return;

    // Clean up textures before renderer
    for (auto& pair : mCoverCache) {
        if (pair.second) SDL_DestroyTexture(pair.second);
    }
    mCoverCache.clear();

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (mRenderer) { SDL_DestroyRenderer(mRenderer); mRenderer = nullptr; }
    if (mWindow) { SDL_DestroyWindow(mWindow); mWindow = nullptr; }
    mRunning = false;
    std::cerr << "ImGuiView::shutdown() complete." << std::endl;
}

bool ImGuiView::isRunning() const { return mRunning; }

void ImGuiView::processEvents() {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) mRunning = false;
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE)
            mRunning = false;
    }
}

// ============================================================================
// Helper: Draw album cover
// ============================================================================

// Helper to create texture from memory
SDL_Texture* ImGuiView::createTextureFromMemory(const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    SDL_RWops* rw = SDL_RWFromConstMem(data.data(), data.size());
    if (!rw) return nullptr;
    
    SDL_Surface* surface = IMG_Load_RW(rw, 1); // 1 = auto-close RWops
    if (!surface) return nullptr;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(mRenderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void ImGuiView::drawAlbumCover(ImDrawList* dl, ImVec2 pos, float size, int trackIndex) {
    bool drawn = false;
    
    if (mController && trackIndex >= 0) {
        // Check cache
        auto it = mCoverCache.find(trackIndex);
        SDL_Texture* texture = nullptr;
        
        if (it != mCoverCache.end()) {
            texture = it->second;
        } else {
            // Load and cache
            auto data = mController->getTrackCoverArt(trackIndex);
            if (!data.empty()) {
                texture = createTextureFromMemory(data);
                if (texture) {
                    mCoverCache[trackIndex] = texture;
                }
            }
        }
        
        if (texture) {
            dl->AddImage((ImTextureID)texture, pos, ImVec2(pos.x + size, pos.y + size));
            drawn = true;
        }
    }

    if (!drawn) {
        // Fallback gradient
        int colorIndex = trackIndex;
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
        
        int idx = abs(colorIndex) % 6;
        dl->AddRectFilledMultiColor(pos, ImVec2(pos.x + size, pos.y + size),
            colors1[idx], colors1[idx], colors2[idx], colors2[idx]);
        dl->AddRect(pos, ImVec2(pos.x + size, pos.y + size), 
            IM_COL32(255, 255, 255, 20), 4.0f, 0, 1.0f);
        
        // Simple circle note
        dl->AddCircle(ImVec2(pos.x + size/2, pos.y + size/2), size/3, IM_COL32(255,255,255,100), 32, 2.0f);
    }
}

// ============================================================================
// Helper: Strip .mp3 extension
// ============================================================================

std::string StripExtension(const std::string& name) {
    if (name.length() > 4 && name.substr(name.length()-4) == ".mp3")
        return name.substr(0, name.length() - 4);
    if (name.length() > 4 && name.substr(name.length()-4) == ".wav")
        return name.substr(0, name.length() - 4);
    return name;
}

// ============================================================================
// Helper: Check if string matches search
// ============================================================================

bool MatchesSearch(const std::string& text, const char* query) {
    if (query[0] == '\0') return true;
    std::string lowerText = text;
    std::string lowerQuery = query;
    std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    return lowerText.find(lowerQuery) != std::string::npos;
}

// ============================================================================
// Helper: Add to Recently Played
// ============================================================================



// ============================================================================
// Main Render
// ============================================================================

void ImGuiView::render() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    SDL_GetWindowSize(mWindow, &mWindowWidth, &mWindowHeight);

    // Layout
    const float rightSidebarW = 300.0f;
    const float playerBarH = 90.0f; // Increased for larger art
    const float gap = 8.0f;
    const float mainW = mWindowWidth - rightSidebarW - gap * 2;
    const float contentH = mWindowHeight - playerBarH - gap;

    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    bool isPlaying = mPlayerState ? mPlayerState->isPlaying() : false;
    
    // Track playback time
    static int lastTrack = -1;
    if (currentTrack != lastTrack) {
        mPlayStartTime = std::chrono::steady_clock::now();
        mPlayStartPos = 0;
        
        lastTrack = currentTrack;
    }

    if (!isPlaying && mWasPlaying) {
         auto now = std::chrono::steady_clock::now();
         mPlayStartPos += (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(now - mPlayStartTime).count();
    }

    if (isPlaying && !mWasPlaying) {
        // Resume from current pos if just paused?
        // Actually g_playStartPos should be updated on pause
        // For now just keep existing logic but add resume support
        mPlayStartTime = std::chrono::steady_clock::now();
    }
    mWasPlaying = isPlaying;

    // Check if song ended (after ~3 min assume ended for demo)
    // TODO: Implement proper callback from AudioPlayer
    
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | 
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar;

    // ========================================================================
    // MAIN CONTENT AREA
    // ========================================================================
    ImGui::SetNextWindowPos(ImVec2(gap, gap));
    ImGui::SetNextWindowSize(ImVec2(mainW, contentH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::SurfaceV);
    
    ImGui::Begin("MainContent", nullptr, flags);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 wp = ImGui::GetWindowPos();
    
    // Background
    dl->AddRectFilled(wp, ImVec2(wp.x + mainW, wp.y + contentH), Colors::Surface, 8.0f);
    
    // ===== SEARCH BAR =====
    ImGui::SetCursorPos(ImVec2(20, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 20.0f);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::HoverV);
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint("##search", "Search songs, artists...", mSearchQuery, 256);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    
    // ===== TAB BUTTONS =====
    ImGui::SetCursorPos(ImVec2(340, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    
    const char* tabs[] = {"All", "Music"}; // Removed Playlist for now
    for (int i = 0; i < 2; i++) {
        if (i > 0) ImGui::SameLine();
        
        bool selected = (mMainTabIndex == i);
        ImGui::PushStyleColor(ImGuiCol_Button, selected ? Colors::WhiteV : Colors::HoverV);
        ImGui::PushStyleColor(ImGuiCol_Text, selected ? Colors::BlackV : Colors::WhiteV);
        
        if (ImGui::Button(tabs[i], ImVec2(80, 28))) mMainTabIndex = i;
        ImGui::PopStyleColor(2);
    }
    ImGui::PopStyleVar();
    
    // ===== TAB CONTENT =====
    ImGui::SetCursorPos(ImVec2(10, 60));  // Below search and tabs
    ImGui::BeginChild("TabContent", ImVec2(mainW - 20, contentH - 70), false);
    
    // Use child window's DrawList for proper clipping
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    
    if (mMainTabIndex == 0) {
        // === ALL TAB (Recently Played) ===
        ImGui::Indent(10);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("Recently Played");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        std::vector<int> history = mController ? mController->getHistory() : std::vector<int>();
        
        if (history.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
            ImGui::Text("No recently played tracks.");
            ImGui::PopStyleColor();
        } else {
            for (int i = (int)history.size() - 1; i >= 0; i--) {
                int trackIdx = history[i];
                if (trackIdx < 0) continue;
                
                std::string tName = mController->getTrackName(trackIdx);
                std::string tArtist = mController->getTrackArtist(trackIdx);
                
                // const RecentTrack& rt = mRecentTracks[i]; // REMOVED
                // bool isCurrent = (currentTrack == (int)rt.index); // REMOVED
                bool isCurrent = (currentTrack == trackIdx);
                
                if (!MatchesSearch(tName, mSearchQuery) && !MatchesSearch(tArtist.c_str(), mSearchQuery)) continue;
                
                ImGui::PushID((int)(2000 + i));
                ImVec2 rowPos = ImGui::GetCursorScreenPos();
                
                // Album cover
                drawAlbumCover(cdl, rowPos, 40, trackIdx);
                
                // Track info
                std::string dispName = StripExtension(tName);
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
                
                // Click to play - overlay
                ImGui::SetCursorScreenPos(rowPos);
                if (ImGui::InvisibleButton("##recent", ImVec2(mainW - 80, 45)) && mController) {
                     mController->playTrack(trackIdx);
                }
                ImGui::PopID();
            }
        }
        ImGui::Unindent(10);
    }
    else if (mMainTabIndex == 1) {
        // === MUSIC TAB (Full Library) ===
        ImGui::Indent(10);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("Music Library (%zu tracks)", mPlaylistDisplay.size());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        for (size_t i = 0; i < mPlaylistDisplay.size(); i++) {
            std::string trackName = mPlaylistDisplay[i];
            
            // Search: Check Name, Artist, and Album
            std::string sArtist = mController ? mController->getTrackArtist(i) : "";
            std::string sAlbum = mController ? mController->getTrackAlbum(i) : "";
            
            bool match = MatchesSearch(trackName, mSearchQuery) || 
                         MatchesSearch(sArtist.c_str(), mSearchQuery) || 
                         MatchesSearch(sAlbum.c_str(), mSearchQuery);
                         
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
            
            // Album cover (offset)
            drawAlbumCover(cdl, ImVec2(rowPos.x + 40, rowPos.y), 40, (int)i);
            
            // Track info
            std::string dispName = StripExtension(trackName);
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
            
            // Add to playlist button "+"
            ImGui::SameLine(mainW - 60);
            ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
            if (ImGui::Button("+##add", ImVec2(25, 25))) {
                // TODO: Show playlist selection popup
            }
            ImGui::PopStyleColor(2);
            
            // Full row clickable
            ImGui::SetCursorScreenPos(rowPos);
            if (ImGui::InvisibleButton("##track", ImVec2(mainW - 80, 45)) && mController) {
                mController->playTrack((int)i);
                mPlayStartTime = std::chrono::steady_clock::now();
                mPlayStartPos = 0;
            }
            
            ImGui::PopID();
        }
        ImGui::Unindent(10);
    }
    else if (mMainTabIndex == 2) {
        // === PLAYLIST TAB ===
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("Your Playlists");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (true) { // Playlist feature disabled for now
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
            ImGui::Text("Playlist feature coming soon!");
            ImGui::PopStyleColor();
        }
    }
    
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();

    // ========================================================================
    // RIGHT SIDEBAR
    // ========================================================================
    ImGui::SetNextWindowPos(ImVec2(mWindowWidth - rightSidebarW - gap, gap));
    ImGui::SetNextWindowSize(ImVec2(rightSidebarW, contentH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::SurfaceV);
    
    ImGui::Begin("RightSidebar", nullptr, flags);
    ImDrawList* rdl = ImGui::GetWindowDrawList();
    ImVec2 rwp = ImGui::GetWindowPos();
    
    rdl->AddRectFilled(rwp, ImVec2(rwp.x + rightSidebarW, rwp.y + contentH), Colors::Surface, 8.0f);
    
    // Tabs
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
    
    // Now Playing
    ImGui::SetCursorPos(ImVec2(15, 55));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Now playing");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(15, 80));
    ImVec2 npPos = ImGui::GetCursorScreenPos();
    
    drawAlbumCover(rdl, npPos, 65, currentTrack >= 0 ? currentTrack : 0);
    
    std::string npName = "No Track Selected";
    if (currentTrack >= 0 && currentTrack < (int)mPlaylistDisplay.size()) {
        npName = StripExtension(mPlaylistDisplay[currentTrack]);
    }
    if (npName.length() > 22) npName = npName.substr(0, 19) + "...";
    
    rdl->AddText(ImVec2(npPos.x + 75, npPos.y + 10), Colors::Green, npName.c_str());
    std::string npArtist = mController && currentTrack >= 0 ? mController->getTrackArtist(currentTrack) : "Unknown Artist";
    std::string npAlbum = mController && currentTrack >= 0 ? mController->getTrackAlbum(currentTrack) : "Unknown Album";
    rdl->AddText(ImVec2(npPos.x + 75, npPos.y + 30), Colors::TextSecondary, npArtist.c_str());
    rdl->AddText(ImVec2(npPos.x + 75, npPos.y + 46), Colors::TextMuted, npAlbum.c_str());
    
    // Queue/Recent list
    ImGui::SetCursorPos(ImVec2(15, 160));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text(mRightTabIndex == 0 ? "Next up" : "Recently played");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(10, 185));
    ImGui::BeginChild("QueueList", ImVec2(rightSidebarW - 20, contentH - 205), false);
    
    if (mRightTabIndex == 0) {
        // Queue
        for (int i = currentTrack + 1; i < (int)mPlaylistDisplay.size() && i < currentTrack + 10; i++) {
            ImGui::PushID(4000 + i);
            ImVec2 tPos = ImGui::GetCursorScreenPos();
            
            drawAlbumCover(rdl, tPos, 40, i);
            
            std::string tName = StripExtension(mPlaylistDisplay[i]);
            if (tName.length() > 20) tName = tName.substr(0, 17) + "...";
            
            rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 5), Colors::White, tName.c_str());
            std::string tArtist = mController ? mController->getTrackArtist(i) : "Unknown Artist";
            rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 22), Colors::TextSecondary, tArtist.c_str());
            
            ImGui::InvisibleButton("##q", ImVec2(rightSidebarW - 40, 45));
            if (ImGui::IsItemClicked() && mController) {
                mController->playTrack(i); // Use playTrack to handle history
            }
            ImGui::PopID();
        }
    } else {
        // Recent
            // Recent List Sidebar
            if (mController) {
                 std::vector<int> history = mController->getHistory();
                 // Show last 10 items
                 for (int i = (int)history.size() - 1; i >= 0 && i >= (int)history.size() - 10; i--) {
                     int trackIdx = history[i];
                     if (trackIdx < 0) continue;
                     
                     ImGui::PushID(5000 + i);
                     ImVec2 tPos = ImGui::GetCursorScreenPos();
                     
                     // Cover Art
                     drawAlbumCover(rdl, tPos, 40, trackIdx);
                     
                     // Track Title
                     std::string tName = mController->getTrackName(trackIdx);
                     tName = StripExtension(tName);
                     if (tName.length() > 20) tName = tName.substr(0, 17) + "...";
                     rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 5), Colors::White, tName.c_str());

                     // Artist
                     std::string tArtist = mController->getTrackArtist(trackIdx);
                     if (tArtist.length() > 25) tArtist = tArtist.substr(0, 22) + "...";
                     rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 22), Colors::TextSecondary, tArtist.c_str());
                     
                     // Clickable Item
                     ImGui::InvisibleButton("##rside", ImVec2(rightSidebarW - 40, 45));
                     if (ImGui::IsItemClicked()) {
                          mController->playTrack(trackIdx);
                     }
                     ImGui::PopID();
                     ImGui::Dummy(ImVec2(0, 5)); // Spacing
                 }
                 if (history.empty()) {
                      ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
                      ImGui::Text("No history yet");
                      ImGui::PopStyleColor();
                 }
            }
    }
    
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();

    // ========================================================================
    // BOTTOM PLAYER BAR
    // ========================================================================
    ImGui::SetNextWindowPos(ImVec2(0, mWindowHeight - playerBarH));
    ImGui::SetNextWindowSize(ImVec2((float)mWindowWidth, playerBarH));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::BlackV);
    
    ImGui::Begin("PlayerBar", nullptr, flags);
    ImDrawList* pdl = ImGui::GetWindowDrawList();
    ImVec2 barPos = ImGui::GetWindowPos();
    
    // === LEFT: Album Art & Info ===
    // Art: 64x64, centered vertically (90 - 64)/2 = 13
    ImVec2 coverPos = ImVec2(barPos.x + 15, barPos.y + 13);
    drawAlbumCover(pdl, coverPos, 64, currentTrack >= 0 ? currentTrack : 0);
    
    // Text Info
    float infoX = coverPos.x + 75;
    float centerX = mWindowWidth / 2.0f;
    
    std::string pTitle = "No Track";
    std::string pArtist = "Unknown Artist";
    std::string pAlbum = "Unknown Album";
    if (currentTrack >= 0 && currentTrack < (int)mPlaylistDisplay.size()) {
        pTitle = StripExtension(mPlaylistDisplay[currentTrack]);
        if (mController) {
             pArtist = mController->getTrackArtist(currentTrack);
             pAlbum = mController->getTrackAlbum(currentTrack);
        }
    }
    
    // Title (Larger, White)
    ImGui::SetCursorPos(ImVec2(infoX, 18));
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::Text("%s", pTitle.c_str());
    ImGui::PopFont();
    
    // Artist (Gray)
    ImGui::SetCursorPos(ImVec2(infoX, 38));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%s", pArtist.c_str());
    
    // Album (Gray, smaller)
    ImGui::SetCursorPos(ImVec2(infoX, 53));
    ImGui::Text("%s", pAlbum.c_str());
    ImGui::PopStyleColor();

    // === CENTER: Controls & Progress ===
    float controlsY = 20.0f;
    
    // Play/Pause Center
    {
        ImVec2 centerBtn = ImVec2(barPos.x + centerX, barPos.y + controlsY + 16);
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
        ImVec2 prevPos = ImVec2(barPos.x + centerX - 50, barPos.y + controlsY + 16);
        pdl->AddTriangleFilled(
            ImVec2(prevPos.x + 6, prevPos.y - 6),
            ImVec2(prevPos.x + 6, prevPos.y + 6),
            ImVec2(prevPos.x - 4, prevPos.y), Colors::TextSecondary);
        pdl->AddRectFilled(ImVec2(prevPos.x - 8, prevPos.y - 6),
                          ImVec2(prevPos.x - 6, prevPos.y + 6), Colors::TextSecondary);
                          
        ImGui::SetCursorPos(ImVec2(centerX - 65, controlsY));
        if (ImGui::InvisibleButton("##prev", ImVec2(30, 30)) && mController) mController->previous();
    }

    // Next Button
    {
         ImVec2 nextPos = ImVec2(barPos.x + centerX + 50, barPos.y + controlsY + 16);
         pdl->AddTriangleFilled(
             ImVec2(nextPos.x - 6, nextPos.y - 6),
             ImVec2(nextPos.x - 6, nextPos.y + 6),
             ImVec2(nextPos.x + 4, nextPos.y), Colors::TextSecondary);
         pdl->AddRectFilled(ImVec2(nextPos.x + 6, nextPos.y - 6),
                           ImVec2(nextPos.x + 8, nextPos.y + 6), Colors::TextSecondary);
                           
         ImGui::SetCursorPos(ImVec2(centerX + 35, controlsY));
         if (ImGui::InvisibleButton("##next", ImVec2(30, 30)) && mController) mController->next();
    }
    
    // Progress Bar
    float progY = 60.0f;
    ImGui::SetCursorPos(ImVec2(centerX - 200, progY));
    
    uint32_t elapsedMs = mPlayStartPos;
    if (isPlaying) {
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
    float progress = (float)elapsedMs / (float)durationMs;
    progress = std::clamp(progress, 0.0f, 1.0f);
    
    // Slider Logic (Seek on Release)
    static float s_dragProgress = 0.0f;
    static bool s_dragging = false;

    float* pValue = s_dragging ? &s_dragProgress : &progress;
    if (!s_dragging) s_dragProgress = progress; // Keeping sync

    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::SurfaceLightV);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, Colors::WhiteV);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::SetNextItemWidth(320);
    
    ImGui::SliderFloat("##prog", pValue, 0, 1, "");
    
    if (ImGui::IsItemActive()) {
        s_dragging = true;
    }
    
    if (ImGui::IsItemDeactivatedAfterEdit()) {
        s_dragging = false;
        if (mController) {
             std::cout << "[DEBUG] Seek triggered on release" << std::endl;
             uint32_t seekPos = (uint32_t)(*pValue * durationMs);
             mController->seek(seekPos);
             mPlayStartPos = seekPos;
             mPlayStartTime = std::chrono::steady_clock::now();
        }
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    // Time Text Right
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%d:%02d", (durationMs/1000)/60, (durationMs/1000)%60);
    ImGui::PopStyleColor();

    // === RIGHT: Volume ===
    float volX = mWindowWidth - 140;
    ImGui::SetCursorPos(ImVec2(volX, 35));
    int vol = mPlayerState ? mPlayerState->getVolume() : 50;
    
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
    if (ImGui::Button("Vol", ImVec2(30, 20)) && mController) mController->toggleMute();
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    ImGui::SetNextItemWidth(80);
    if (ImGui::SliderInt("##vol", &vol, 0, 100, "") && mController) mController->setVolume(vol);

    ImGui::End();
    ImGui::PopStyleColor();

    // Render
    ImGui::Render();
    SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 255);
    SDL_RenderClear(mRenderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), mRenderer);
    SDL_RenderPresent(mRenderer);
}

// Stubs
void ImGuiView::renderMenuBar() {}
void ImGuiView::renderTransportControls() {}
void ImGuiView::renderVolumeControl() {}
void ImGuiView::renderProgressBar() {}
void ImGuiView::renderPlaylist() {}
void ImGuiView::renderSerialPanel() {}
void ImGuiView::renderStatusBar() {}

void ImGuiView::addPlaylistItem(const std::string& trackName) {
    mPlaylistDisplay.push_back(trackName);
}

void ImGuiView::clearPlaylistDisplay() {
    mPlaylistDisplay.clear();
}

} // namespace View
