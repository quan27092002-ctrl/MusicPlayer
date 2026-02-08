/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/view/MockLifecycleManager.h
 * DESCRIPTION: GoogleMock implementation for ILifecycleManager interface.
 */

#ifndef MOCK_LIFECYCLE_MANAGER_H
#define MOCK_LIFECYCLE_MANAGER_H

#include <gmock/gmock.h>
#include "view/imguiview/interfaces/ILifecycleManager.h"

namespace View {

class MockLifecycleManager : public ILifecycleManager {
public:
    MOCK_METHOD(bool, initialize, (int width, int height, const std::string& title), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(void, processEvents, (bool& running), (override));
    MOCK_METHOD(void, beginFrame, (), (override));
    MOCK_METHOD(void, endFrame, (), (override));
    MOCK_METHOD(SDL_Window*, getWindow, (), (const, override));
    MOCK_METHOD(SDL_Renderer*, getRenderer, (), (const, override));
    MOCK_METHOD(int, getWindowWidth, (), (const, override));
    MOCK_METHOD(int, getWindowHeight, (), (const, override));
};

} // namespace View

#endif // MOCK_LIFECYCLE_MANAGER_H
