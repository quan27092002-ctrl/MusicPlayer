/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/MainContent.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Main content area component (SRP).
 */

#ifndef MAINCONTENT_H
#define MAINCONTENT_H

#include "../interfaces/IWindowComponent.h"
#include "../AssetManager.h"
#include "../../../controller/IAppController.h"
#include "../../../model/IPlayerState.h"
#include <memory>
#include <vector>
#include <string>

namespace View {

class MainContent : public IWindowComponent {
public:
    MainContent(std::shared_ptr<Controller::IAppController> controller,
                std::shared_ptr<Model::IPlayerState> playerState,
                std::shared_ptr<AssetManager> assetManager);
    
    void render() override;
    
    /**
     * @brief Update the internal playlist display copy.
     * @param playlist List of track names
     */
    void setPlaylist(const std::vector<std::string>& playlist);

private:
    std::shared_ptr<Controller::IAppController> mController;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    std::shared_ptr<AssetManager> mAssetManager;
    
    std::vector<std::string> mPlaylistDisplay;
    char mSearchQuery[256];
    int mMainTabIndex;

    std::string stripExtension(const std::string& name);
    bool matchesSearch(const std::string& text, const char* query);
    
    void renderSearch();
    void renderTabs();
    void renderRecentTab(float mainW, float contentH);
    void renderMusicTab(float mainW, float contentH);
    void renderPlaylistTab();
    void renderPlaylistDetailView(); // Detail View renderer

    // UI State
    bool mShowPlaylistDetail;
    int mSelectedPlaylistIndex;
};

} // namespace View

#endif // MAINCONTENT_H
