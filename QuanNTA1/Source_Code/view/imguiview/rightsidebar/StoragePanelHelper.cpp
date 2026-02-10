/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/sidebar/StoragePanelHelper.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of StoragePanelHelper.
 */

#include "StoragePanelHelper.h"
#include <cstdio>

namespace View {

void StoragePanelHelper::refresh(std::shared_ptr<Controller::IAppController> controller) {
    if (!controller) return;
    
    mDevices = controller->getStorageDevices();
    
    // Select first by default if available
    if (!mDevices.empty()) {
        if (mSelectedIndex < 0 || mSelectedIndex >= (int)mDevices.size()) {
            mSelectedIndex = 0;
        }
    } else {
        mSelectedIndex = -1;
    }

    if (mSelectedIndex >= 0) {
        snprintf(mBuffer, sizeof(mBuffer), "%s", mDevices[mSelectedIndex].name.c_str());
    } else {
        snprintf(mBuffer, sizeof(mBuffer), "No drives found");
    }
}

std::string StoragePanelHelper::getSelectedPath() const {
    if (mSelectedIndex >= 0 && mSelectedIndex < (int)mDevices.size()) {
        return mDevices[mSelectedIndex].path;
    }
    return "";
}

} // namespace View
