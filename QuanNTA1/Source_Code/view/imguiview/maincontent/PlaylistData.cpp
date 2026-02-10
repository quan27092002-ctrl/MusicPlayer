/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/playlist/PlaylistData.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of PlaylistData.
 */

#include "PlaylistData.h"

namespace View {

PlaylistData PlaylistManager::sEmptyPlaylist = {"", "", 0, {}};

PlaylistManager::PlaylistManager() 
    : mDefaultPlaylistsPopulated(false)
{
    // Initialize with default playlists
    mPlaylists = {
        { "Chill Vibes", "Relaxing acoustic & lofi tracks", 0, {} },
        { "High Energy", "Workout & Upbeat hits", 1, {} },
        { "Focus Flow", "Instrumental study mix", 2, {} }
    };
}

void PlaylistManager::createPlaylist(const std::string& name, const std::string& desc, int colorIdx) {
    if (name.empty()) return;
    
    PlaylistData pd;
    pd.name = name;
    pd.desc = desc;
    pd.colorIdx = colorIdx;
    
    mPlaylists.push_back(pd);
}

void PlaylistManager::updatePlaylist(int idx, const std::string& name, const std::string& desc, int colorIdx) {
    if (idx < 0 || idx >= (int)mPlaylists.size()) return;
    if (name.empty()) return;
    
    mPlaylists[idx].name = name;
    mPlaylists[idx].desc = desc;
    mPlaylists[idx].colorIdx = colorIdx;
}

void PlaylistManager::deletePlaylist(int idx) {
    if (idx >= 0 && idx < (int)mPlaylists.size()) {
        mPlaylists.erase(mPlaylists.begin() + idx);
    }
}

void PlaylistManager::addTrackToPlaylist(int playlistIdx, const std::string& trackPath) {
    if (playlistIdx >= 0 && playlistIdx < (int)mPlaylists.size() && !trackPath.empty()) {
        mPlaylists[playlistIdx].trackPaths.push_back(trackPath);
    }
}

void PlaylistManager::removeTrackFromPlaylist(int playlistIdx, size_t trackIdx) {
    if (playlistIdx >= 0 && playlistIdx < (int)mPlaylists.size()) {
        auto& paths = mPlaylists[playlistIdx].trackPaths;
        if (trackIdx < paths.size()) {
            paths.erase(paths.begin() + trackIdx);
        }
    }
}

const PlaylistData& PlaylistManager::getPlaylist(int idx) const {
    if (idx < 0 || idx >= (int)mPlaylists.size()) return sEmptyPlaylist;
    return mPlaylists[idx];
}

PlaylistData& PlaylistManager::getPlaylistMutable(int idx) {
    if (idx < 0 || idx >= (int)mPlaylists.size()) return sEmptyPlaylist;
    return mPlaylists[idx];
}

void PlaylistManager::populateDefaultPlaylists(std::shared_ptr<Controller::IAppController> controller) {
    if (mDefaultPlaylistsPopulated || !controller) return;
    
    size_t totalTracks = controller->getLibrarySize();
    if (totalTracks > 0) {
        // Distribute tracks: 0-9 to Chill, 10-19 to High Energy, 20-29 to Focus
        for (int i = 0; i < 3 && i < (int)mPlaylists.size(); ++i) {
            int startIdx = i * 10;
            for (int j = 0; j < 10; ++j) {
                int trackIdx = startIdx + j;
                if (trackIdx < (int)totalTracks) {
                    std::string path = controller->getLibraryTrackPath(trackIdx);
                    if (!path.empty()) {
                        mPlaylists[i].trackPaths.push_back(path);
                    }
                }
            }
        }
        mDefaultPlaylistsPopulated = true;
    }
}

} // namespace View
