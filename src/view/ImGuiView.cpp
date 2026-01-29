/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of Dear ImGui View using SDL2 + SDL_Renderer.
 */

#include "ImGuiView.h"

// ImGui core
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_sdl2.h"
#include "imgui/backends/imgui_impl_sdlrenderer2.h"

#include <cstdio>

namespace View {

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
    , mWindowWidth(800)
    , mWindowHeight(600) {
}

ImGuiView::~ImGuiView() {
    shutdown();
}

// ============================================================================
// IView Interface
// ============================================================================

bool ImGuiView::initialize() {
    // Initialize SDL video (audio already initialized by AudioPlayer)
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }

    // Create window
    mWindow = SDL_CreateWindow(
        "S32K Media Player",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        mWindowWidth, mWindowHeight,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (!mWindow) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        return false;
    }

    // Create renderer
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

    // Setup ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup style
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 3.0f;
    style.FramePadding = ImVec2(8, 4);

    // Initialize backends
    ImGui_ImplSDL2_InitForSDLRenderer(mWindow, mRenderer);
    ImGui_ImplSDLRenderer2_Init(mRenderer);

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
    // Start new frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // Get window size for layout
    SDL_GetWindowSize(mWindow, &mWindowWidth, &mWindowHeight);

    // Main window (fullscreen docked)
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2((float)mWindowWidth, (float)mWindowHeight));
    
    ImGuiWindowFlags windowFlags = 
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar;

    ImGui::Begin("MainWindow", nullptr, windowFlags);

    renderMenuBar();
    
    // Layout: Left panel (controls), Right panel (playlist)
    ImGui::BeginChild("ControlPanel", ImVec2(mWindowWidth * 0.6f, -50), true);
    
    renderTransportControls();
    ImGui::Separator();
    renderVolumeControl();
    ImGui::Separator();
    renderProgressBar();
    ImGui::Separator();
    renderSerialPanel();
    
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("PlaylistPanel", ImVec2(0, -50), true);
    renderPlaylist();
    ImGui::EndChild();

    // Status bar at bottom
    renderStatusBar();

    ImGui::End();

    // Render
    ImGui::Render();
    SDL_SetRenderDrawColor(mRenderer, 30, 30, 30, 255);
    SDL_RenderClear(mRenderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), mRenderer);
    SDL_RenderPresent(mRenderer);
}

// ============================================================================
// UI Components
// ============================================================================

void ImGuiView::renderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Add Track...")) {
                // TODO: File dialog
            }
            if (ImGui::MenuItem("Clear Playlist")) {
                if (mController) mController->clearPlaylist();
                clearPlaylistDisplay();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                mRunning = false;
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}

void ImGuiView::renderTransportControls() {
    ImGui::Text("Transport Controls");
    ImGui::Spacing();

    // Get current state
    bool isPlaying = mPlayerState ? mPlayerState->isPlaying() : false;
    Model::PlaybackState state = mPlayerState ? 
        mPlayerState->getPlaybackState() : Model::PlaybackState::STOPPED;

    // Button size
    ImVec2 btnSize(60, 40);

    // Previous button
    if (ImGui::Button("<<", btnSize)) {
        if (mController) mController->previous();
    }
    ImGui::SameLine();

    // Play/Pause button
    const char* playPauseLabel = isPlaying ? "||" : ">";
    if (ImGui::Button(playPauseLabel, btnSize)) {
        if (mController) {
            if (isPlaying) mController->pause();
            else mController->play();
        }
    }
    ImGui::SameLine();

    // Stop button
    if (ImGui::Button("[]", btnSize)) {
        if (mController) mController->stop();
    }
    ImGui::SameLine();

    // Next button
    if (ImGui::Button(">>", btnSize)) {
        if (mController) mController->next();
    }

    // Show current state
    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
    
    const char* stateText = "Unknown";
    ImVec4 stateColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    
    switch (state) {
        case Model::PlaybackState::STOPPED:
            stateText = "STOPPED";
            stateColor = ImVec4(0.8f, 0.2f, 0.2f, 1.0f);
            break;
        case Model::PlaybackState::PLAYING:
            stateText = "PLAYING";
            stateColor = ImVec4(0.2f, 0.8f, 0.2f, 1.0f);
            break;
        case Model::PlaybackState::PAUSED:
            stateText = "PAUSED";
            stateColor = ImVec4(0.8f, 0.8f, 0.2f, 1.0f);
            break;
    }
    
    ImGui::TextColored(stateColor, "%s", stateText);
}

void ImGuiView::renderVolumeControl() {
    ImGui::Text("Volume");
    ImGui::Spacing();

    int volume = mPlayerState ? mPlayerState->getVolume() : 50;
    bool isMuted = mPlayerState ? mPlayerState->isMuted() : false;

    // Mute button
    const char* muteLabel = isMuted ? "Unmute" : "Mute";
    if (ImGui::Button(muteLabel, ImVec2(80, 30))) {
        if (mController) mController->toggleMute();
    }
    ImGui::SameLine();

    // Volume slider
    ImGui::SetNextItemWidth(-1);
    if (ImGui::SliderInt("##Volume", &volume, 0, 100, "%d%%")) {
        if (mController) mController->setVolume(volume);
    }

    // Visual indicator if muted
    if (isMuted) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "(MUTED)");
    }
}

void ImGuiView::renderProgressBar() {
    ImGui::Text("Progress");
    ImGui::Spacing();

    // Get position/duration from player state
    uint32_t position = mPlayerState ? mPlayerState->getCurrentPosition() : 0;
    // Duration would come from AudioPlayer, using placeholder for now
    uint32_t duration = 180000;  // 3 minutes placeholder

    float progress = duration > 0 ? (float)position / (float)duration : 0.0f;

    // Format time
    int posMin = (position / 1000) / 60;
    int posSec = (position / 1000) % 60;
    int durMin = (duration / 1000) / 60;
    int durSec = (duration / 1000) % 60;

    char timeText[32];
    snprintf(timeText, sizeof(timeText), "%02d:%02d / %02d:%02d", 
             posMin, posSec, durMin, durSec);

    ImGui::ProgressBar(progress, ImVec2(-1, 20), timeText);
}

void ImGuiView::renderPlaylist() {
    ImGui::Text("Playlist");
    ImGui::Separator();

    int currentTrack = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;

    for (size_t i = 0; i < mPlaylistDisplay.size(); ++i) {
        bool isSelected = (static_cast<int>(i) == currentTrack);
        
        if (ImGui::Selectable(mPlaylistDisplay[i].c_str(), isSelected)) {
            // TODO: Load and play this track
        }
    }

    if (mPlaylistDisplay.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
                          "(No tracks in playlist)");
    }
}

void ImGuiView::renderSerialPanel() {
    ImGui::Text("S32K Connection");
    ImGui::Spacing();

    bool isConnected = mController ? mController->isConnectedToBoard() : false;
    
    if (isConnected) {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Connected");
        ImGui::SameLine();
        if (ImGui::Button("Disconnect")) {
            if (mController) mController->disconnectFromBoard();
        }
    } else {
        ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Disconnected");
        
        static char portName[64] = "/dev/ttyUSB0";
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("Port", portName, sizeof(portName));
        
        ImGui::SameLine();
        if (ImGui::Button("Connect")) {
            if (mController) mController->connectToBoard(portName, 115200);
        }
    }
}

void ImGuiView::renderStatusBar() {
    ImGui::Separator();
    
    int trackIndex = mPlayerState ? mPlayerState->getCurrentTrackIndex() : -1;
    int volume = mPlayerState ? mPlayerState->getVolume() : 0;
    bool isConnected = mController ? mController->isConnectedToBoard() : false;
    
    ImGui::Text("Track: %d | Volume: %d%% | Board: %s",
                trackIndex,
                volume,
                isConnected ? "Connected" : "Disconnected");
}

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
