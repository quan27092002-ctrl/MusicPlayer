/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/components/sidebar/ConnectionPanelHelper.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Connection panel logic extracted from RightSidebar (SRP).
 */

#ifndef CONNECTIONPANELHELPER_H
#define CONNECTIONPANELHELPER_H

#include <vector>
#include <string>
#include <memory>
#include "../../../../controller/IAppController.h"

namespace View {

/**
 * @brief Helper class for Connection Panel UI logic.
 * 
 * Extracted from RightSidebar to follow SRP.
 * Manages serial port selection and connection.
 */
class ConnectionPanelHelper {
public:
    // Refresh available ports
    void refresh(std::shared_ptr<Controller::IAppController> controller);
    
    // Get selected port name
    std::string getSelectedPort() const;
    
    // Accessors
    int getSelectedIndex() const { return mSelectedIndex; }
    void setSelectedIndex(int idx);
    const std::vector<std::string>& getPorts() const { return mPorts; }
    const char* getPortBuffer() const { return mPortBuffer; }

private:
    std::vector<std::string> mPorts;
    int mSelectedIndex = -1;
    char mPortBuffer[128] = "";
};

} // namespace View

#endif // CONNECTIONPANELHELPER_H
