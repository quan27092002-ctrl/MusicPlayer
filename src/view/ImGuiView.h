/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Dear ImGui-based View implementation using SDL2 + SDL_Renderer.
 */

#ifndef IMGUIVIEW_H
#define IMGUIVIEW_H

#include "IView.h"
#include "../controller/IAppController.h"
#include "../model/IPlayerState.h"
#include "imgui/imgui.h"

#include <chrono>
#include <SDL.h>
#include <memory>
#include <string>
#include <vector>
#include <map>

namespace View {

/**
 * @brief Dear ImGui View implementation.
 * 
 * Creates a window with transport controls, volume slider,
 * playlist display, and status information.
 */
class ImGuiView : public IView {
public:

private:
    // Dependencies
    std::shared_ptr<Controller::IAppController> mController;
    std::shared_ptr<Model::IPlayerState> mPlayerState;

    // SDL resources
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;

    // State
    bool mRunning;
    int mWindowWidth;
    int mWindowHeight;
    int mLastTrackIndex = -1; // To track changes for Recent list
    
    // Playlist cache (for display)
    std::vector<std::string> mPlaylistDisplay;

    
    // UI State
    int mMainTabIndex = 1;  // 0=All, 1=Music, 2=Playlist
    int mRightTabIndex = 0; // 0=Queue, 1=Recent
    char mSearchQuery[256] = "";

    // Playback Tracking
    std::chrono::steady_clock::time_point mPlayStartTime;
    uint32_t mPlayStartPos = 0;
    bool mWasPlaying = false;
    
    // Texture Cache
    std::map<int, SDL_Texture*> mCoverCache;

    // Helper to create texture from memory
    SDL_Texture* createTextureFromMemory(const std::vector<uint8_t>& data);
    void addToRecentlyPlayed(size_t index, const std::string& name);
    
    void renderMenuBar();
    void renderTransportControls();
    void renderVolumeControl();
    void renderProgressBar();
    void renderPlaylist();
    void renderStatusBar();
    void renderSerialPanel();
    
    // Album Art Helper
    void drawAlbumCover(ImDrawList* dl, ImVec2 pos, float size, int trackIndex);

public:
    /**
     * @brief Constructor with dependency injection.
     * @param controller App controller for sending commands
     * @param playerState Player state for reading current state
     */
    ImGuiView(
        std::shared_ptr<Controller::IAppController> controller,
        std::shared_ptr<Model::IPlayerState> playerState
    );

    /**
     * @brief Destructor.
     */
    ~ImGuiView() override;

    // Delete copy
    ImGuiView(const ImGuiView&) = delete;
    ImGuiView& operator=(const ImGuiView&) = delete;

    // ========================================================================
    // IView Interface
    // ========================================================================

    bool initialize() override;
    void shutdown() override;
    bool isRunning() const override;
    void processEvents() override;
    void render() override;

    // ========================================================================
    // Additional Methods
    // ========================================================================

    /**
     * @brief Add a track name to the playlist display.
     * @param trackName Name to display
     */
    void addPlaylistItem(const std::string& trackName);

    /**
     * @brief Clear the playlist display.
     */
    void clearPlaylistDisplay();
};

} // namespace View

#endif // IMGUIVIEW_H
