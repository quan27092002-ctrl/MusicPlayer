/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.cpp
 * DESCRIPTION: Spotify-style UI with all features.
 */

#include "ImGuiView.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include <cstdio>
#include <cmath>
#include <algorithm>
#include <chrono>

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

// Track info for recently played
struct RecentTrack {
    std::string name;
    std::string artist;
    std::string album;
    size_t index;
    std::chrono::steady_clock::time_point playedAt;
};

// User playlist
struct UserPlaylist {
    std::string name;
    std::vector<size_t> trackIndices;
};

// Global state
static std::vector<RecentTrack> g_recentlyPlayed;
static std::vector<UserPlaylist> g_userPlaylists;
static int g_mainTabIndex = 1;  // 0=All, 1=Music, 2=Playlist
static int g_rightTabIndex = 0; // 0=Queue, 1=Recent
static char g_searchQuery[256] = "";
static bool g_loopEnabled = false;
static bool g_shuffleEnabled = false;

// Playback tracking
static std::chrono::steady_clock::time_point g_playStartTime;
static uint32_t g_playStartPos = 0;
static bool g_wasPlaying = false;

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
    , mWindowWidth(1280)
    , mWindowHeight(850) {
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
    c[ImGuiCol_FrameBgActive] = Colors::GreenV;
    c[ImGuiCol_ScrollbarBg] = Colors::TransparentV;
    c[ImGuiCol_ScrollbarGrab] = Colors::HoverV;
    c[ImGuiCol_SliderGrab] = Colors::GreenV;
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.15f, 0.9f, 0.45f, 1);
    c[ImGuiCol_Button] = Colors::SurfaceLightV;
    c[ImGuiCol_ButtonHovered] = Colors::HoverV;
    c[ImGuiCol_ButtonActive] = Colors::GreenV;
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
        "S32K Media Player",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        mWindowWidth, mWindowHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
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
    if (!mWindow) return;
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    if (mRenderer) { SDL_DestroyRenderer(mRenderer); mRenderer = nullptr; }
    if (mWindow) { SDL_DestroyWindow(mWindow); mWindow = nullptr; }
    mRunning = false;
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

void DrawAlbumCover(ImDrawList* dl, ImVec2 pos, float size, int colorIndex) {
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

void AddToRecentlyPlayed(std::shared_ptr<Controller::IAppController> controller, size_t index, const std::string& name) {
    // Remove if already exists
    g_recentlyPlayed.erase(
        std::remove_if(g_recentlyPlayed.begin(), g_recentlyPlayed.end(),
            [index](const RecentTrack& t) { return t.index == index; }),
        g_recentlyPlayed.end());
    
    // Add to front
    RecentTrack rt;
    rt.name = name;
    rt.artist = controller ? controller->getTrackArtist(index) : "Unknown Artist";
    rt.album = "Unknown Album";
    rt.index = index;
    rt.playedAt = std::chrono::steady_clock::now();
    g_recentlyPlayed.insert(g_recentlyPlayed.begin(), rt);
    
    // Limit size
    if (g_recentlyPlayed.size() > 20) g_recentlyPlayed.pop_back();
}

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
    const float playerBarH = 60.0f;
    const float gap = 8.0f;
    const float mainW = mWindowWidth - rightSidebarW - gap * 2;
    const float contentH = mWindowHeight - playerBarH - gap;

    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    bool isPlaying = mPlayerState ? mPlayerState->isPlaying() : false;
    
    // Track playback time
    if (isPlaying && !g_wasPlaying) {
        g_playStartTime = std::chrono::steady_clock::now();
        g_playStartPos = 0;
    }
    g_wasPlaying = isPlaying;

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
    ImGui::InputTextWithHint("##search", "Search songs, artists...", g_searchQuery, 256);
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    
    // ===== TAB BUTTONS =====
    ImGui::SetCursorPos(ImVec2(340, 15));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
    
    const char* tabs[] = {"All", "Music", "Playlist"};
    for (int i = 0; i < 3; i++) {
        if (i > 0) ImGui::SameLine();
        
        bool selected = (g_mainTabIndex == i);
        ImGui::PushStyleColor(ImGuiCol_Button, selected ? Colors::WhiteV : Colors::HoverV);
        ImGui::PushStyleColor(ImGuiCol_Text, selected ? Colors::BlackV : Colors::WhiteV);
        
        if (ImGui::Button(tabs[i], ImVec2(80, 28))) g_mainTabIndex = i;
        ImGui::PopStyleColor(2);
    }
    ImGui::PopStyleVar();
    
    // ===== TAB CONTENT =====
    ImGui::SetCursorPos(ImVec2(10, 60));  // Below search and tabs
    ImGui::BeginChild("TabContent", ImVec2(mainW - 20, contentH - 70), false);
    
    // Use child window's DrawList for proper clipping
    ImDrawList* cdl = ImGui::GetWindowDrawList();
    
    if (g_mainTabIndex == 0) {
        // === ALL TAB (Recently Played) ===
        ImGui::Indent(10);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("Recently Played");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        if (g_recentlyPlayed.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
            ImGui::Text("No recently played tracks. Play a song to see it here.");
            ImGui::PopStyleColor();
        } else {
            for (size_t i = 0; i < g_recentlyPlayed.size(); i++) {
                const RecentTrack& rt = g_recentlyPlayed[i];
                bool isCurrent = (currentTrack == (int)rt.index);
                
                if (!MatchesSearch(rt.name, g_searchQuery)) continue;
                
                ImGui::PushID((int)(2000 + i));
                ImVec2 rowPos = ImGui::GetCursorScreenPos();
                
                // Album cover
                DrawAlbumCover(cdl, rowPos, 40, (int)rt.index);
                
                // Track info
                std::string dispName = StripExtension(rt.name);
                if (dispName.length() > 35) dispName = dispName.substr(0, 32) + "...";
                
                ImGui::SetCursorPosX(60);
                ImGui::PushStyleColor(ImGuiCol_Text, isCurrent ? Colors::GreenV : Colors::WhiteV);
                ImGui::Text("%s", dispName.c_str());
                ImGui::PopStyleColor();
                ImGui::SetCursorPosX(60);
                ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextSecV);
                ImGui::Text("%s", rt.artist.c_str());
                ImGui::PopStyleColor();
                
                // Click to play - overlay
                ImGui::SetCursorScreenPos(rowPos);
                if (ImGui::InvisibleButton("##recent", ImVec2(mainW - 80, 45)) && mController) {
                    mController->loadTrack(mController->getTrackPath(rt.index));
                    mController->play();
                    if (mPlayerState) mPlayerState->setCurrentTrackIndex((int)rt.index);
                }
                ImGui::PopID();
            }
        }
        ImGui::Unindent(10);
    }
    else if (g_mainTabIndex == 1) {
        // === MUSIC TAB (Full Library) ===
        ImGui::Indent(10);
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("Music Library (%zu tracks)", mPlaylistDisplay.size());
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        for (size_t i = 0; i < mPlaylistDisplay.size(); i++) {
            std::string trackName = mPlaylistDisplay[i];
            if (!MatchesSearch(trackName, g_searchQuery)) continue;
            
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
            DrawAlbumCover(cdl, ImVec2(rowPos.x + 40, rowPos.y), 40, (int)i);
            
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
            ImGui::Text("%s", artistStr.c_str());
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
                mController->loadTrack(mController->getTrackPath(i));
                mController->play();
                if (mPlayerState) mPlayerState->setCurrentTrackIndex((int)i);
                if (mPlayerState) mPlayerState->setCurrentTrackIndex((int)i);
                AddToRecentlyPlayed(mController, i, trackName);
                g_playStartTime = std::chrono::steady_clock::now();
                g_playStartPos = 0;
            }
            
            ImGui::PopID();
        }
        ImGui::Unindent(10);
    }
    else if (g_mainTabIndex == 2) {
        // === PLAYLIST TAB ===
        ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
        ImGui::Text("Your Playlists");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        
        // Create playlist button
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 16.0f);
        if (ImGui::Button("+ Create Playlist", ImVec2(150, 30))) {
            UserPlaylist newPl;
            newPl.name = "Playlist " + std::to_string(g_userPlaylists.size() + 1);
            g_userPlaylists.push_back(newPl);
        }
        ImGui::PopStyleVar();
        ImGui::Spacing();
        
        if (g_userPlaylists.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
            ImGui::Text("No playlists yet. Create one to get started!");
            ImGui::PopStyleColor();
        } else {
            for (size_t p = 0; p < g_userPlaylists.size(); p++) {
                UserPlaylist& pl = g_userPlaylists[p];
                
                ImGui::PushID((int)(3000 + p));
                ImVec2 plPos = ImGui::GetCursorScreenPos();
                
                // Playlist card
                dl->AddRectFilled(plPos, ImVec2(plPos.x + 200, plPos.y + 60),
                    Colors::SurfaceLight, 8.0f);
                dl->AddText(ImVec2(plPos.x + 15, plPos.y + 10), Colors::White, pl.name.c_str());
                
                char countStr[32];
                snprintf(countStr, 32, "%zu tracks", pl.trackIndices.size());
                dl->AddText(ImVec2(plPos.x + 15, plPos.y + 32), Colors::TextSecondary, countStr);
                
                ImGui::InvisibleButton("##playlist", ImVec2(200, 65));
                ImGui::PopID();
            }
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
    
    ImGui::PushStyleColor(ImGuiCol_Button, g_rightTabIndex == 0 ? Colors::GreenV : Colors::HoverV);
    ImGui::PushStyleColor(ImGuiCol_Text, g_rightTabIndex == 0 ? Colors::BlackV : Colors::WhiteV);
    if (ImGui::Button("Queue", ImVec2(80, 26))) g_rightTabIndex = 0;
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Button, g_rightTabIndex == 1 ? Colors::GreenV : Colors::HoverV);
    ImGui::PushStyleColor(ImGuiCol_Text, g_rightTabIndex == 1 ? Colors::BlackV : Colors::WhiteV);
    if (ImGui::Button("Recent", ImVec2(80, 26))) g_rightTabIndex = 1;
    ImGui::PopStyleColor(2);
    
    ImGui::PopStyleVar();
    
    // Now Playing
    ImGui::SetCursorPos(ImVec2(15, 55));
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::WhiteV);
    ImGui::Text("Now playing");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(15, 80));
    ImVec2 npPos = ImGui::GetCursorScreenPos();
    
    DrawAlbumCover(rdl, npPos, 65, currentTrack >= 0 ? currentTrack : 0);
    
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
    ImGui::Text(g_rightTabIndex == 0 ? "Next up" : "Recently played");
    ImGui::PopStyleColor();
    
    ImGui::SetCursorPos(ImVec2(10, 185));
    ImGui::BeginChild("QueueList", ImVec2(rightSidebarW - 20, contentH - 205), false);
    
    if (g_rightTabIndex == 0) {
        // Queue
        for (int i = currentTrack + 1; i < (int)mPlaylistDisplay.size() && i < currentTrack + 10; i++) {
            ImGui::PushID(4000 + i);
            ImVec2 tPos = ImGui::GetCursorScreenPos();
            
            DrawAlbumCover(rdl, tPos, 40, i);
            
            std::string tName = StripExtension(mPlaylistDisplay[i]);
            if (tName.length() > 20) tName = tName.substr(0, 17) + "...";
            
            rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 5), Colors::White, tName.c_str());
            std::string tArtist = mController ? mController->getTrackArtist(i) : "Unknown Artist";
            rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 22), Colors::TextSecondary, tArtist.c_str());
            
            ImGui::InvisibleButton("##q", ImVec2(rightSidebarW - 40, 45));
            if (ImGui::IsItemClicked() && mController) {
                mController->loadTrack(mController->getTrackPath(i));
                mController->play();
                if (mPlayerState) mPlayerState->setCurrentTrackIndex(i);
                if (mPlayerState) mPlayerState->setCurrentTrackIndex(i);
                AddToRecentlyPlayed(mController, i, mPlaylistDisplay[i]);
            }
            ImGui::PopID();
        }
    } else {
        // Recent
        for (size_t i = 0; i < g_recentlyPlayed.size() && i < 10; i++) {
            const RecentTrack& rt = g_recentlyPlayed[i];
            bool isCurr = (currentTrack == (int)rt.index);
            
            ImGui::PushID((int)(5000 + i));
            ImVec2 tPos = ImGui::GetCursorScreenPos();
            
            DrawAlbumCover(rdl, tPos, 40, (int)rt.index);
            
            std::string tName = StripExtension(rt.name);
            if (tName.length() > 20) tName = tName.substr(0, 17) + "...";
            
            rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 5), 
                isCurr ? Colors::Green : Colors::White, tName.c_str());
            rdl->AddText(ImVec2(tPos.x + 50, tPos.y + 22), Colors::TextSecondary, rt.artist.c_str());
            
            ImGui::InvisibleButton("##r", ImVec2(rightSidebarW - 40, 45));
            if (ImGui::IsItemClicked() && mController) {
                mController->loadTrack(mController->getTrackPath(rt.index));
                mController->play();
                if (mPlayerState) mPlayerState->setCurrentTrackIndex((int)rt.index);
            }
            ImGui::PopID();
        }
        
        if (g_recentlyPlayed.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
            ImGui::Text("No history yet");
            ImGui::PopStyleColor();
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
    
    // === LEFT: Track Info ===
    ImVec2 coverPos = ImVec2(barPos.x + 12, barPos.y + 10);
    DrawAlbumCover(pdl, coverPos, 60, currentTrack >= 0 ? currentTrack : 0);
    
    std::string playerTrack = "No Track";
    if (currentTrack >= 0 && currentTrack < (int)mPlaylistDisplay.size()) {
        playerTrack = StripExtension(mPlaylistDisplay[currentTrack]);
    }
    if (playerTrack.length() > 25) playerTrack = playerTrack.substr(0, 22) + "...";
    
    pdl->AddText(ImVec2(coverPos.x + 70, coverPos.y + 10), Colors::White, playerTrack.c_str());
    std::string pArtist = mController && currentTrack >= 0 ? mController->getTrackArtist(currentTrack) : "Unknown Artist";
    std::string pAlbum = mController && currentTrack >= 0 ? mController->getTrackAlbum(currentTrack) : "Unknown Album";
    pdl->AddText(ImVec2(coverPos.x + 70, coverPos.y + 28), Colors::TextSecondary, pArtist.c_str());
    pdl->AddText(ImVec2(coverPos.x + 70, coverPos.y + 44), Colors::TextMuted, pAlbum.c_str());
    
    // === CENTER: Playback Controls ===
    float centerX = mWindowWidth / 2.0f;
    
    // Shuffle button
    ImGui::SetCursorPos(ImVec2(centerX - 95, 20));
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
    ImGui::PushStyleColor(ImGuiCol_Text, g_shuffleEnabled ? Colors::GreenV : Colors::TextSecV);
    if (ImGui::Button("S##shuf", ImVec2(26, 26))) g_shuffleEnabled = !g_shuffleEnabled;
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    // Previous button - Draw triangle
    {
        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(btnPos.x + 13, btnPos.y + 13);
        
        // Draw |<< icon
        pdl->AddRectFilled(ImVec2(center.x - 8, center.y - 6), 
                          ImVec2(center.x - 6, center.y + 6), Colors::TextSecondary);
        pdl->AddTriangleFilled(
            ImVec2(center.x + 6, center.y - 6),
            ImVec2(center.x + 6, center.y + 6),
            ImVec2(center.x - 4, center.y),
            Colors::TextSecondary);
        
        ImGui::InvisibleButton("##prev", ImVec2(26, 26));
        if (ImGui::IsItemClicked() && mController) mController->previous();
    }
    
    ImGui::SameLine();
    
    // Play/Pause button
    {
        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(btnPos.x + 18, btnPos.y + 18);
        
        bool hovered = ImGui::IsMouseHoveringRect(
            ImVec2(center.x - 18, center.y - 18),
            ImVec2(center.x + 18, center.y + 18));
        
        pdl->AddCircleFilled(center, 18, hovered ? Colors::TextSecondary : Colors::White, 32);
        
        if (isPlaying) {
            pdl->AddRectFilled(ImVec2(center.x - 5, center.y - 6), 
                              ImVec2(center.x - 1, center.y + 6), Colors::Black);
            pdl->AddRectFilled(ImVec2(center.x + 1, center.y - 6), 
                              ImVec2(center.x + 5, center.y + 6), Colors::Black);
        } else {
            pdl->AddTriangleFilled(
                ImVec2(center.x - 4, center.y - 7),
                ImVec2(center.x - 4, center.y + 7),
                ImVec2(center.x + 7, center.y),
                Colors::Black);
        }
        
        ImGui::InvisibleButton("##play", ImVec2(36, 36));
        if (ImGui::IsItemClicked() && mController) {
            if (isPlaying) mController->pause();
            else mController->play();
        }
    }
    
    ImGui::SameLine();
    
    // Next button
    {
        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(btnPos.x + 13, btnPos.y + 13);
        
        pdl->AddTriangleFilled(
            ImVec2(center.x - 6, center.y - 6),
            ImVec2(center.x - 6, center.y + 6),
            ImVec2(center.x + 4, center.y),
            Colors::TextSecondary);
        pdl->AddRectFilled(ImVec2(center.x + 6, center.y - 6), 
                          ImVec2(center.x + 8, center.y + 6), Colors::TextSecondary);
        
        ImGui::InvisibleButton("##next", ImVec2(26, 26));
        if (ImGui::IsItemClicked() && mController) mController->next();
    }
    
    ImGui::SameLine();
    
    // Loop button
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
    ImGui::PushStyleColor(ImGuiCol_Text, g_loopEnabled ? Colors::GreenV : Colors::TextSecV);
    if (ImGui::Button("R##loop", ImVec2(26, 26))) g_loopEnabled = !g_loopEnabled;
    ImGui::PopStyleColor(2);
    
    // Progress bar
    ImGui::SetCursorPos(ImVec2(centerX - 180, 52));
    
    // Calculate position based on elapsed time
    uint32_t elapsedMs = 0;
    if (isPlaying) {
        auto now = std::chrono::steady_clock::now();
        elapsedMs = g_playStartPos + (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>(
            now - g_playStartTime).count();
    }
    
    uint32_t durationMs = 0;
    if (mController && currentTrack >= 0) {
        durationMs = mController->getTrackDuration(currentTrack) * 1000;
    }
    if (durationMs == 0) durationMs = 180000; // Fallback to 3 min if 0
    int posM = (elapsedMs / 1000) / 60;
    int posS = (elapsedMs / 1000) % 60;
    int durM = (durationMs / 1000) / 60;
    int durS = (durationMs / 1000) % 60;
    
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%d:%02d", posM, posS);
    ImGui::PopStyleColor();
    
    ImGui::SameLine();
    
    float progress = (float)elapsedMs / (float)durationMs;
    progress = std::clamp(progress, 0.0f, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::SurfaceLightV);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, Colors::WhiteV);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // Thinner slider
    ImGui::SetNextItemWidth(280);
    if (ImGui::SliderFloat("##prog", &progress, 0, 1, "")) {
        uint32_t seekPos = (uint32_t)(progress * durationMs);
        g_playStartPos = seekPos;
        g_playStartTime = std::chrono::steady_clock::now();
        // Seek audio to new position
        if (mController) mController->seek(seekPos);
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_Text, Colors::TextMutedV);
    ImGui::Text("%d:%02d", durM, durS);
    ImGui::PopStyleColor();
    
    // === RIGHT: Volume ===
    float rightX = mWindowWidth - 160;
    ImGui::SetCursorPos(ImVec2(rightX, 28));
    
    int vol = mPlayerState ? mPlayerState->getVolume() : 50;
    bool muted = mPlayerState ? mPlayerState->isMuted() : false;
    
    ImGui::PushStyleColor(ImGuiCol_Button, Colors::TransparentV);
    ImGui::PushStyleColor(ImGuiCol_Text, muted ? Colors::TextMutedV : Colors::TextSecV);
    if (ImGui::Button(muted ? "X" : "V", ImVec2(24, 24))) {
        if (mController) mController->toggleMute();
    }
    ImGui::PopStyleColor(2);
    
    ImGui::SameLine();
    
    ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::SurfaceLightV);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, Colors::WhiteV);
    ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize, 6.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0)); // Thinner slider
    ImGui::SetNextItemWidth(90);
    if (ImGui::SliderInt("##vol", &vol, 0, 100, "")) {
        if (mController) mController->setVolume(vol);
    }
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
    
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
