/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Facade implementation of ImGuiView.
 */

#include "ImGuiView.h"
#include <iostream>
#include "imgui/backends/imgui_impl_sdlrenderer2.h"
#include "imgui/imgui.h"

namespace View {

ImGuiView::ImGuiView(std::shared_ptr<Controller::IAppController> controller,
                     std::shared_ptr<Model::IPlayerState> playerState)
    : mController(controller)
    , mPlayerState(playerState)
    , mRunning(false)
{
    // Lifecycle manager created first
    mLifecycle = std::make_unique<LifecycleManager>();
}

ImGuiView::~ImGuiView() {
    shutdown();
}

bool ImGuiView::initialize() {
    if (!mLifecycle->initialize(1080, 720, "Media Player")) {
        return false;
    }

    // Asset manager needs renderer from lifecycle
    mAssetManager = std::make_shared<AssetManager>(mLifecycle->getRenderer());

    // Initialize UI Components
    mMainContent = std::make_unique<MainContent>(mController, mPlayerState, mAssetManager);
    mRightSidebar = std::make_unique<RightSidebar>(mController, mPlayerState, mAssetManager);
    mPlayerBar = std::make_unique<PlayerBar>(mController, mPlayerState, mAssetManager);

    // Initial playlist load
    updatePlaylistData();

    mRunning = true;
    return true;
}

void ImGuiView::shutdown() {
    std::cerr << "ImGuiView::shutdown() called." << std::endl;
    
    // Release components that hold asset manager references
    mMainContent.reset();
    mRightSidebar.reset();
    mPlayerBar.reset();
    
    // Clear assets
    if (mAssetManager) {
        mAssetManager->clearCache();
        mAssetManager.reset();
    }
    
    // Shutdown lifecycle
    if (mLifecycle) {
        mLifecycle->shutdown();
    }
    
    mRunning = false;
    std::cerr << "ImGuiView::shutdown() complete." << std::endl;
}

bool ImGuiView::isRunning() const {
    return mRunning;
}

void ImGuiView::processEvents() {
    if (mLifecycle) {
        mLifecycle->processEvents(mRunning);
    }
}

void ImGuiView::render() {
    if (!mLifecycle) return;

    mLifecycle->beginFrame();
    
    // Update data if needed (simple check: size mismatch)
    // Real app might use observer pattern or dirty flag
    static size_t lastSize = 0;
    size_t currentSize = mController ? mController->getPlaylistSize() : 0;
    if (currentSize != lastSize) {
        updatePlaylistData();
        lastSize = currentSize;
    }
    
    // Render Components
    if (mMainContent) mMainContent->render();
    if (mRightSidebar) mRightSidebar->render();
    
    // PlayerBar needs update logic call
    if (mPlayerBar) {
        // We can expose update if needed, but render handles it internally mostly
        mPlayerBar->update(mLifecycle->getWindowWidth(), mLifecycle->getWindowHeight());
        mPlayerBar->render();
    }
    
    mLifecycle->endFrame(); 
    
    // Draw data is still handled normally by ImGui::Render() inside standard Loop? 
    // Wait, LifecycleManager logic needs to match original flow
    // Original ImGuiView.cpp:
    // ImGui::Render();
    // SDL_RenderClear(mRenderer);
    // ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData());
    // SDL_RenderPresent(mRenderer);
    
    // Let's add that logic here or in LifecycleManager.
    // For separation, LifecycleManager should handle the raw SDL calls.
    
    // Actually, LifecycleManager::endFrame was empty in my prev impl.
    // Let's fix that - but wait, I can't modify LifecycleManager easily now without another tool call.
    // I will implement the render pass here in the facade for clarity/control.
    
    static const ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.00f);
    
    ImGui::Render();
    SDL_Renderer* renderer = mLifecycle->getRenderer();
    SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
    SDL_RenderClear(renderer);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}

void ImGuiView::updatePlaylistData() {
    if (!mController) return;
    
    std::vector<std::string> playlist;
    size_t count = mController->getPlaylistSize();
    for (size_t i = 0; i < count; ++i) {
        playlist.push_back(mController->getTrackName(i));
    }
    
    if (mMainContent) mMainContent->setPlaylist(playlist);
    if (mRightSidebar) mRightSidebar->setPlaylist(playlist);
}

} // namespace View
