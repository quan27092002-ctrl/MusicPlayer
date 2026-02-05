/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/IAppController.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Aggregate interface for the main application controller.
 *              Combines multiple small interfaces for backward compatibility.
 */

#ifndef IAPPCONTROLLER_H
#define IAPPCONTROLLER_H

#include "appcontroller/interfaces/IAppLifecycle.h"
#include "appcontroller/interfaces/IPlaybackController.h"
#include "appcontroller/interfaces/IVolumeController.h"
#include "appcontroller/interfaces/IPlaylistManager.h"
#include "appcontroller/interfaces/IHistoryManager.h"
#include "appcontroller/interfaces/IBoardCommunicator.h"

namespace Controller {

/**
 * @brief Aggregate interface for the main application controller.
 * 
 * Combines IAppLifecycle, IPlaybackController, IVolumeController,
 * IPlaylistManager, IHistoryManager, and IBoardCommunicator interfaces.
 * This provides backward compatibility while following ISP.
 * 
 * Clients that need all features can depend on this interface,
 * while clients needing specific features can depend on smaller interfaces.
 */
class IAppController : public IAppLifecycle,
                       public IPlaybackController,
                       public IVolumeController,
                       public IPlaylistManager,
                       public IHistoryManager,
                       public IBoardCommunicator {
public:
    /**
     * @brief Play a track by index in current queue.
     * @param index Index in the playlist
     */
    virtual void playTrack(int index) = 0;

    /**
     * @brief Play a list of tracks (replaces current queue).
     * @param filePaths Vector of file paths
     */
    virtual void playPlaylist(const std::vector<std::string>& filePaths) = 0;
    
    /**
     * @brief Queue a track to play next.
     * @param filePath Path to file
     */
    virtual void queueNext(const std::string& filePath) = 0;

    virtual ~IAppController() = default;
};

} // namespace Controller

#endif // IAPPCONTROLLER_H
