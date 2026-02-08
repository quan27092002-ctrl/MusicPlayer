/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/interfaces/IHistoryManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for history navigation operations.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IHISTORYMANAGER_H
#define IHISTORYMANAGER_H

#include <vector>
#include <memory> 

namespace Model {
    class MediaFile;
}

namespace Controller {

/**
 * @brief Interface for history navigation operations.
 * 
 * This interface provides access to playback history for
 * navigating back to previously played tracks.
 */
class IHistoryManager {
public:
    virtual ~IHistoryManager() = default;

    /**
     * @brief Get the history of played tracks.
     * @return Vector of track indices in play order
     */
    virtual std::vector<int> getHistory() const = 0;
    
    /**
     * @brief Get the number of items in history.
     */
    virtual size_t getHistorySize() const = 0;

    /**
     * @brief Get a history item by index.
     * @param index Index into history stack (0 = oldest).
     * @return Smart pointer to MediaFile.
     */
    virtual std::string getHistoryTrackPath(size_t index) const = 0;
    virtual std::shared_ptr<Model::MediaFile> getHistoryItem(size_t index) const = 0;
};

} // namespace Controller

#endif // IHISTORYMANAGER_H
