/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/RightSidebar.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Right sidebar component (SRP).
 */

#ifndef RIGHTSIDEBAR_H
#define RIGHTSIDEBAR_H

#include "../interfaces/IWindowComponent.h"
#include "../AssetManager.h"
#include "../../../controller/IAppController.h"
#include "../../../model/IPlayerState.h"
#include <memory>
#include <vector>
#include <string>

namespace View {

class RightSidebar : public IWindowComponent {
public:
    RightSidebar(std::shared_ptr<Controller::IAppController> controller,
                 std::shared_ptr<Model::IPlayerState> playerState,
                 std::shared_ptr<AssetManager> assetManager);
    
    void render() override;
    
    void setPlaylist(const std::vector<std::string>& playlist);

private:
    std::shared_ptr<Controller::IAppController> mController;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    std::shared_ptr<AssetManager> mAssetManager;
    
    std::vector<std::string> mPlaylistDisplay;
    int mRightTabIndex; // 0=Queue, 1=Recent

    std::string stripExtension(const std::string& name);
    
    void renderNowPlaying(ImDrawList* dl);
    void renderTabs();
    void renderQueue(ImDrawList* dl, float width, int currentTrack);
    void renderRecent(ImDrawList* dl, float width);
    void renderConnectionPanel();

    char mPortBuffer[64] = "/dev/ttyUSB0"; // Default/Fallback
    std::vector<std::string> mAvailablePorts;
    int mSelectedPortIndex = -1;
    void refreshPorts();
};

} // namespace View

#endif // RIGHTSIDEBAR_H
