/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/playlist/PlaylistData.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Data structures and manager for playlist operations (SRP).
 */

#ifndef PLAYLISTDATA_H
#define PLAYLISTDATA_H

#include <string>
#include <vector>
#include <memory>
#include "../../../controller/IAppController.h"

namespace View {

/**
 * @brief Data structure for a single playlist.
 */
struct PlaylistData {
    std::string name;
    std::string desc;
    int colorIdx;
    std::vector<std::string> trackPaths;
};

/**
 * @brief Manages playlist data and CRUD operations.
 * 
 * Extracted from MainContent to follow SRP.
 * Handles session-based playlist storage.
 */
class PlaylistManager {
public:
    PlaylistManager();
    
    // CRUD Operations
    void createPlaylist(const std::string& name, const std::string& desc = "", int colorIdx = 0);
    void updatePlaylist(int idx, const std::string& name, const std::string& desc, int colorIdx);
    void deletePlaylist(int idx);
    void addTrackToPlaylist(int playlistIdx, const std::string& trackPath);
    void removeTrackFromPlaylist(int playlistIdx, size_t trackIdx);
    
    // Accessors
    size_t getPlaylistCount() const { return mPlaylists.size(); }
    const PlaylistData& getPlaylist(int idx) const;
    PlaylistData& getPlaylistMutable(int idx);
    const std::vector<PlaylistData>& getAllPlaylists() const { return mPlaylists; }
    
    // Auto-populate default playlists from library
    void populateDefaultPlaylists(std::shared_ptr<Controller::IAppController> controller);
    bool isDefaultPopulated() const { return mDefaultPlaylistsPopulated; }

private:
    std::vector<PlaylistData> mPlaylists;
    bool mDefaultPlaylistsPopulated;
    
    static PlaylistData sEmptyPlaylist;
};

} // namespace View

#endif // PLAYLISTDATA_H
