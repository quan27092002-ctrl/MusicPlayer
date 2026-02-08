/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/sidebar/ConnectionPanelHelper.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of ConnectionPanelHelper.
 */

#include "ConnectionPanelHelper.h"
#include <cstdio>

namespace View {

void ConnectionPanelHelper::refresh(std::shared_ptr<Controller::IAppController> controller) {
    if (!controller) return;
    
    mPorts = controller->getAvailablePorts();
    
    // Try to keep selection or select first
    if (!mPorts.empty()) {
        if (mSelectedIndex < 0 || mSelectedIndex >= (int)mPorts.size()) {
            mSelectedIndex = 0;
        }
    } else {
        mSelectedIndex = -1;
    }
    
    // Update buffer
    if (mSelectedIndex >= 0 && mSelectedIndex < (int)mPorts.size()) {
        snprintf(mPortBuffer, sizeof(mPortBuffer), "%s", mPorts[mSelectedIndex].c_str());
    }
}

void ConnectionPanelHelper::setSelectedIndex(int idx) {
    mSelectedIndex = idx;
    if (idx >= 0 && idx < (int)mPorts.size()) {
        snprintf(mPortBuffer, sizeof(mPortBuffer), "%s", mPorts[idx].c_str());
    }
}

std::string ConnectionPanelHelper::getSelectedPort() const {
    if (mSelectedIndex >= 0 && mSelectedIndex < (int)mPorts.size()) {
        return mPorts[mSelectedIndex];
    }
    return "";
}

} // namespace View
