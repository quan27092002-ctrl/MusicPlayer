/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/sidebar/StoragePanelHelper.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Storage panel logic extracted from RightSidebar (SRP).
 */

#ifndef STORAGEPANELHELPER_H
#define STORAGEPANELHELPER_H

#include <vector>
#include <string>
#include <memory>
#include "../../../../controller/IAppController.h"
#include "../../../../controller/IStorageManager.h"

namespace View {

/**
 * @brief Helper class for Storage Panel UI logic.
 * 
 * Extracted from RightSidebar to follow SRP.
 * Manages storage device selection and loading.
 */
class StoragePanelHelper {
public:
    // Refresh available storage devices
    void refresh(std::shared_ptr<Controller::IAppController> controller);
    
    // Get selected storage path (for loading)
    std::string getSelectedPath() const;
    
    // Accessors
    int getSelectedIndex() const { return mSelectedIndex; }
    void setSelectedIndex(int idx) { mSelectedIndex = idx; }
    const std::vector<Controller::StorageDevice>& getDevices() const { return mDevices; }
    const char* getDisplayBuffer() const { return mBuffer; }

private:
    std::vector<Controller::StorageDevice> mDevices;
    int mSelectedIndex = -1;
    char mBuffer[128] = "Scanning...";
};

} // namespace View

#endif // STORAGEPANELHELPER_H
