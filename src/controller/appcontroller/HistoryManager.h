/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/HistoryManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of IHistoryManager.
 *              Follows Single Responsibility Principle (SRP).
 */

#ifndef HISTORYMANAGER_IMPL_H
#define HISTORYMANAGER_IMPL_H

#include "interfaces/IHistoryManager.h"
#include "../../model/MediaFile.h"
#include <vector>
#include <list>
#include <memory>
#include <mutex>

namespace Controller {

/**
 * @brief Concrete implementation of IHistoryManager.
 * 
 * Manages playback history for navigation.
 * Thread-safe implementation.
 */
class HistoryManagerImpl : public IHistoryManager {
public:
    using MediaFilePtr = std::shared_ptr<Model::MediaFile>;
    
    HistoryManagerImpl();
    ~HistoryManagerImpl() override = default;
    
    // IHistoryManager interface
    std::vector<int> getHistory() const override;
    
    // Additional methods for internal use
    void pushHistory(MediaFilePtr track);
    MediaFilePtr popHistory();
    void clearHistory();
    
    // Set playlist reference for index lookup
    void setPlaylistRef(const std::list<MediaFilePtr>* playlist, std::mutex* mutex);
    
private:
    std::vector<MediaFilePtr> mHistoryStack;
    const std::list<MediaFilePtr>* mPlaylistRef;
    std::mutex* mPlaylistMutex;
    mutable std::mutex mMutex;
    static constexpr size_t MAX_HISTORY_SIZE = 50;
};

} // namespace Controller

#endif // HISTORYMANAGER_IMPL_H
