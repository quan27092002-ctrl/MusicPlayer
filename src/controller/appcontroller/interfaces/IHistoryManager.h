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
};

} // namespace Controller

#endif // IHISTORYMANAGER_H
