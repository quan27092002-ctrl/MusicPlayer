/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/PlayerBar.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Bottom player bar component (SRP).
 */

#ifndef PLAYERBAR_H
#define PLAYERBAR_H

#include "../interfaces/IWindowComponent.h"
#include "../AssetManager.h"
#include "../../../controller/IAppController.h"
#include "../../../model/IPlayerState.h"
#include <memory>
#include <chrono>

namespace View {

class PlayerBar : public IWindowComponent {
public:
    PlayerBar(std::shared_ptr<Controller::IAppController> controller,
              std::shared_ptr<Model::IPlayerState> playerState,
              std::shared_ptr<AssetManager> assetManager);
    
    void render() override;
    
    // Updates internal play position logic
    void update(float windowWidth, float windowHeight);

private:
    std::shared_ptr<Controller::IAppController> mController;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    std::shared_ptr<AssetManager> mAssetManager;
    
    // State tracking for smooth slider
    bool mWasPlaying;
    std::chrono::time_point<std::chrono::steady_clock> mPlayStartTime;
    uint32_t mPlayStartPos;
    std::string mLastTrackPath;
    int mLastTrack;
    bool mIsDraggingSlider;
    
    std::string stripExtension(const std::string& name);
};

} // namespace View

#endif // PLAYERBAR_H
