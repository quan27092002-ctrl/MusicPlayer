/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/ImGuiView.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Main View class implementing IView (Facade).
 *              Uses composition to manage UI components.
 */

#ifndef IMGUIVIEW_H
#define IMGUIVIEW_H

#include "IView.h"
#include "../controller/IAppController.h"
#include "../model/IPlayerState.h"
#include "imguiview/managers/LifecycleManager.h"
#include "imguiview/managers/AssetManager.h"
#include "imguiview/maincontent/MainContent.h"
#include "imguiview/rightsidebar/RightSidebar.h"
#include "imguiview/playerbar/PlayerBar.h"
#include <memory>
#include <vector>
#include <string>

namespace View {

class ImGuiView : public IView {
public:
    ImGuiView(std::shared_ptr<Controller::IAppController> controller,
              std::shared_ptr<Model::IPlayerState> playerState);
    ~ImGuiView() override;

    // IView Interface
    bool initialize() override;
    void shutdown() override;
    bool isRunning() const override;
    void processEvents() override;
    void render() override;

private:
    std::shared_ptr<Controller::IAppController> mController;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    
    // Subsystems
    std::unique_ptr<LifecycleManager> mLifecycle;
    std::shared_ptr<AssetManager> mAssetManager; // Shared with components
    
    // UI Components
    std::unique_ptr<MainContent> mMainContent;
    std::unique_ptr<RightSidebar> mRightSidebar;
    std::unique_ptr<PlayerBar> mPlayerBar;

    bool mRunning;
    
    // Helper to sync playlist data to components
    void updatePlaylistData();
    
    // Thread safety for updates
    std::atomic<bool> mPlaylistDirty;
};

} // namespace View

#endif // IMGUIVIEW_H
