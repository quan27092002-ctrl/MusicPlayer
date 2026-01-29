/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Spotify-style Dear ImGui View using SDL2 + SDL_Renderer.
 */

#include "ImGuiView.h"

// ImGui core
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"

#include <cstdio>
#include <cmath>

namespace View {

// Spotify-like colors
namespace SpotifyColors {
    const ImVec4 Background      = ImVec4(0.07f, 0.07f, 0.07f, 1.0f);  // #121212
    const ImVec4 Sidebar         = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);     // #000000
    const ImVec4 PlayerBar       = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);  // #1C1C1C
    const ImVec4 CardBg          = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);  // #1C1C1C
    const ImVec4 CardHover       = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);  // #282828
    const ImVec4 Green           = ImVec4(0.12f, 0.84f, 0.38f, 1.0f);  // #1DB954
    const ImVec4 GreenHover      = ImVec4(0.14f, 0.90f, 0.42f, 1.0f);  // #1ED760
    const ImVec4 TextPrimary     = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);     // White
    const ImVec4 TextSecondary   = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);  // #B3B3B3
    const ImVec4 TextMuted       = ImVec4(0.40f, 0.40f, 0.40f, 1.0f);  // #666666
    const ImVec4 SliderBg        = ImVec4(0.30f, 0.30f, 0.30f, 1.0f);
    const ImVec4 SliderActive    = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
}

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
    , mWindowWidth(1200)
    , mWindowHeight(800) {
}

ImGuiView::~ImGuiView() {
    shutdown();
}

// ============================================================================
// Spotify Theme Setup
// ============================================================================

void setupSpotifyTheme() {
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Rounding
    style.WindowRounding = 0.0f;
    style.ChildRounding = 8.0f;
    style.FrameRounding = 20.0f;
    style.PopupRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 20.0f;
    style.TabRounding = 4.0f;
    
    // Padding
    style.WindowPadding = ImVec2(0, 0);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(8, 8);
    style.ItemInnerSpacing = ImVec2(8, 8);
    style.ScrollbarSize = 8.0f;
    
    // Colors
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = SpotifyColors::Background;
    colors[ImGuiCol_ChildBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_PopupBg] = SpotifyColors::CardBg;
    colors[ImGuiCol_Border] = ImVec4(0.1f, 0.1f, 0.1f, 0.5f);
    
    colors[ImGuiCol_FrameBg] = SpotifyColors::SliderBg;
    colors[ImGuiCol_FrameBgHovered] = SpotifyColors::CardHover;
    colors[ImGuiCol_FrameBgActive] = SpotifyColors::Green;
    
    colors[ImGuiCol_TitleBg] = SpotifyColors::Sidebar;
    colors[ImGuiCol_TitleBgActive] = SpotifyColors::Sidebar;
    
    colors[ImGuiCol_MenuBarBg] = SpotifyColors::Sidebar;
    
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_ScrollbarGrab] = SpotifyColors::SliderBg;
    colors[ImGuiCol_ScrollbarGrabHovered] = SpotifyColors::TextSecondary;
    colors[ImGuiCol_ScrollbarGrabActive] = SpotifyColors::TextPrimary;
    
    colors[ImGuiCol_SliderGrab] = SpotifyColors::TextPrimary;
    colors[ImGuiCol_SliderGrabActive] = SpotifyColors::Green;
    
    colors[ImGuiCol_Button] = SpotifyColors::CardBg;
    colors[ImGuiCol_ButtonHovered] = SpotifyColors::CardHover;
    colors[ImGuiCol_ButtonActive] = SpotifyColors::Green;
    
    colors[ImGuiCol_Header] = SpotifyColors::CardBg;
    colors[ImGuiCol_HeaderHovered] = SpotifyColors::CardHover;
    colors[ImGuiCol_HeaderActive] = SpotifyColors::Green;
    
    colors[ImGuiCol_Text] = SpotifyColors::TextPrimary;
    colors[ImGuiCol_TextDisabled] = SpotifyColors::TextMuted;
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
        "S32K Media Player - Spotify Style",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        mWindowWidth, mWindowHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!mWindow) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

    mRenderer = SDL_CreateRenderer(
        mWindow,
        -1,
        SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
    );

    if (!mRenderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // Load a larger font for better look
    io.Fonts->AddFontDefault();

    ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
    ImGui_ImplSDLRenderer2_Init(mRenderer);

    // Apply Spotify theme
    setupSpotifyTheme();

    mRunning = true;
    return true;
}

void ImGuiView::shutdown() {
    if (!mWindow) return;

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (mRenderer) {
        SDL_DestroyRenderer(mRenderer);
        mRenderer = nullptr;
    }

    if (mWindow) {
        SDL_DestroyWindow(mWindow);
        mWindow = nullptr;
    }

    mRunning = false;
}

bool ImGuiView::isRunning() const {
    return mRunning;
}

void ImGuiView::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);

        if (event.type == SDL_QUIT) {
            mRunning = false;
        }

        if (event.type == SDL_WINDOWEVENT &&
            event.window.event == SDL_WINDOWEVENT_CLOSE &&
            event.window.windowID == SDL_GetWindowID(mWindow)) {
            mRunning = false;
        }
    }
}

void ImGuiView::render() {
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    SDL_GetWindowSize(mWindow, &mWindowWidth, &mWindowHeight);

    // Layout constants
    const float sidebarWidth = 240.0f;
    const float playerBarHeight = 90.0f;
    const float mainContentWidth = mWindowWidth - sidebarWidth;
    const float mainContentHeight = mWindowHeight - playerBarHeight;

    // ========================================================================
    // LEFT SIDEBAR (Playlist)
    // ========================================================================
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, mainContentHeight));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, SpotifyColors::Sidebar);
    
    ImGuiWindowFlags sidebarFlags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("Sidebar", nullptr, sidebarFlags);
    
    // Logo / Title
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextPrimary);
    ImGui::SetCursorPos(ImVec2(20, 20));
    ImGui::Text("S32K Player");
    ImGui::PopStyleColor();
    
    // Playlist header
    ImGui::SetCursorPos(ImVec2(20, 60));
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextSecondary);
    ImGui::Text("YOUR LIBRARY");
    ImGui::PopStyleColor();
    
    // Playlist items
    ImGui::SetCursorPos(ImVec2(8, 90));
    ImGui::BeginChild("PlaylistScroll", ImVec2(sidebarWidth - 16, mainContentHeight - 100), false);
    
    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    
    for (size_t i = 0; i < mPlaylistDisplay.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == currentTrack);
        
        // Highlight current track
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::Green);
        } else {
            ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextSecondary);
        }
        
        // Selectable track
        char label[256];
        snprintf(label, sizeof(label), "  %zu. %s", i + 1, mPlaylistDisplay[i].c_str());
        
        if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_None, ImVec2(sidebarWidth - 24, 28))) {
            if (mController) {
                std::string path = mController->getTrackPath(i);
                mController->loadTrack(path);
                mController->play();
                if (mPlayerState) {
                    mPlayerState->setCurrentTrackIndex(static_cast<int>(i));
                }
            }
        }
        
        ImGui::PopStyleColor();
    }
    
    if (mPlaylistDisplay.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextMuted);
        ImGui::Text("  No tracks loaded");
        ImGui::PopStyleColor();
    }
    
    ImGui::EndChild();
    ImGui::End();
    ImGui::PopStyleColor();

    // ========================================================================
    // MAIN CONTENT AREA (Now Playing)
    // ========================================================================
    ImGui::SetNextWindowPos(ImVec2(sidebarWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(mainContentWidth, mainContentHeight));
    
    ImGuiWindowFlags mainFlags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("MainContent", nullptr, mainFlags);
    
    // Center content
    float centerX = mainContentWidth / 2;
    float centerY = mainContentHeight / 2 - 50;
    
    // Now Playing title
    ImGui::SetCursorPos(ImVec2(centerX - 100, centerY - 100));
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextSecondary);
    ImGui::Text("NOW PLAYING");
    ImGui::PopStyleColor();
    
    // Track name
    std::string trackName = "No Track Selected";
    if (currentTrack >= 0 && currentTrack < (int)mPlaylistDisplay.size()) {
        trackName = mPlaylistDisplay[currentTrack];
    }
    
    // Calculate text position for centering
    ImVec2 textSize = ImGui::CalcTextSize(trackName.c_str());
    ImGui::SetCursorPos(ImVec2(centerX - textSize.x / 2, centerY - 40));
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextPrimary);
    ImGui::Text("%s", trackName.c_str());
    ImGui::PopStyleColor();
    
    // Playback state
    Model::PlaybackState state = mPlayerState ? 
        mPlayerState->getPlaybackState() : Model::PlaybackState::STOPPED;
    
    const char* stateText = "Stopped";
    ImVec4 stateColor = SpotifyColors::TextMuted;
    
    switch (state) {
        case Model::PlaybackState::PLAYING:
            stateText = "Playing";
            stateColor = SpotifyColors::Green;
            break;
        case Model::PlaybackState::PAUSED:
            stateText = "Paused";
            stateColor = SpotifyColors::TextSecondary;
            break;
        case Model::PlaybackState::STOPPED:
            stateText = "Stopped";
            stateColor = SpotifyColors::TextMuted;
            break;
    }
    
    ImVec2 stateSize = ImGui::CalcTextSize(stateText);
    ImGui::SetCursorPos(ImVec2(centerX - stateSize.x / 2, centerY));
    ImGui::TextColored(stateColor, "%s", stateText);
    
    // Track count info
    char infoText[64];
    snprintf(infoText, sizeof(infoText), "Track %d of %zu", 
             currentTrack + 1, mPlaylistDisplay.size());
    ImVec2 infoSize = ImGui::CalcTextSize(infoText);
    ImGui::SetCursorPos(ImVec2(centerX - infoSize.x / 2, centerY + 40));
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextMuted);
    ImGui::Text("%s", infoText);
    ImGui::PopStyleColor();
    
    ImGui::End();

    // ========================================================================
    // BOTTOM PLAYER BAR
    // ========================================================================
    ImGui::SetNextWindowPos(ImVec2(0, mainContentHeight));
    ImGui::SetNextWindowSize(ImVec2((float)mWindowWidth, playerBarHeight));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, SpotifyColors::PlayerBar);
    
    ImGuiWindowFlags playerFlags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("PlayerBar", nullptr, playerFlags);
    
    // Left section: Current track info
    ImGui::SetCursorPos(ImVec2(20, 25));
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextPrimary);
    
    std::string shortName = trackName;
    if (shortName.length() > 30) {
        shortName = shortName.substr(0, 27) + "...";
    }
    ImGui::Text("%s", shortName.c_str());
    ImGui::PopStyleColor();
    
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextSecondary);
    ImGui::Text("Unknown Artist");
    ImGui::PopStyleColor();
    ImGui::EndGroup();
    
    // Center section: Playback controls
    float controlsX = mWindowWidth / 2.0f - 100;
    ImGui::SetCursorPos(ImVec2(controlsX, 20));
    
    bool isPlaying = mPlayerState ? mPlayerState->isPlaying() : false;
    
    // Previous button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SpotifyColors::CardHover);
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextSecondary);
    
    if (ImGui::Button("<<", ImVec2(40, 40))) {
        if (mController) mController->previous();
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    
    // Play/Pause button (Green circle)
    ImGui::PushStyleColor(ImGuiCol_Button, SpotifyColors::Green);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SpotifyColors::GreenHover);
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::Sidebar);
    
    const char* playPauseLabel = isPlaying ? "||" : ">";
    if (ImGui::Button(playPauseLabel, ImVec2(50, 50))) {
        if (mController) {
            if (isPlaying) mController->pause();
            else mController->play();
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    
    // Next button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SpotifyColors::CardHover);
    ImGui::PushStyleColor(ImGuiCol_Text, SpotifyColors::TextSecondary);
    
    if (ImGui::Button(">>", ImVec2(40, 40))) {
        if (mController) mController->next();
    }
    ImGui::PopStyleColor(3);
    
    // Progress bar
    ImGui::SetCursorPos(ImVec2(controlsX - 100, 65));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, SpotifyColors::SliderBg);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, SpotifyColors::TextPrimary);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, SpotifyColors::Green);
    
    static float progress = 0.0f;
    ImGui::SetNextItemWidth(400);
    ImGui::SliderFloat("##Progress", &progress, 0.0f, 1.0f, "");
    ImGui::PopStyleColor(3);
    
    // Right section: Volume control
    float volumeX = mWindowWidth - 180;
    ImGui::SetCursorPos(ImVec2(volumeX, 35));
    
    int volume = mPlayerState ? mPlayerState->getVolume() : 50;
    bool isMuted = mPlayerState ? mPlayerState->isMuted() : false;
    
    // Mute button
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, SpotifyColors::CardHover);
    ImGui::PushStyleColor(ImGuiCol_Text, isMuted ? SpotifyColors::Green : SpotifyColors::TextSecondary);
    
    if (ImGui::Button(isMuted ? "X" : "V", ImVec2(30, 30))) {
        if (mController) mController->toggleMute();
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    
    // Volume slider
    ImGui::PushStyleColor(ImGuiCol_FrameBg, SpotifyColors::SliderBg);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, SpotifyColors::TextPrimary);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, SpotifyColors::Green);
    
    ImGui::SetNextItemWidth(100);
    if (ImGui::SliderInt("##Volume", &volume, 0, 100, "")) {
        if (mController) mController->setVolume(volume);
    }
    ImGui::PopStyleColor(3);
    
    ImGui::End();
    ImGui::PopStyleColor();

    // Render
    ImGui::Render();
    SDL_SetRenderDrawColor(mRenderer, 18, 18, 18, 255);
    SDL_RenderClear(mRenderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), mRenderer);
    SDL_RenderPresent(mRenderer);
}

// ============================================================================
// UI Components (Kept for compatibility, replaced by render())
// ============================================================================

void ImGuiView::renderMenuBar() {}
void ImGuiView::renderTransportControls() {}
void ImGuiView::renderVolumeControl() {}
void ImGuiView::renderProgressBar() {}
void ImGuiView::renderPlaylist() {}
void ImGuiView::renderSerialPanel() {}
void ImGuiView::renderStatusBar() {}

// ============================================================================
// Additional Methods
// ============================================================================

void ImGuiView::addPlaylistItem(const std::string& trackName) {
    mPlaylistDisplay.push_back(trackName);
}

void ImGuiView::clearPlaylistDisplay() {
    mPlaylistDisplay.clear();
}

} // namespace View
