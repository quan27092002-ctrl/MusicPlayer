/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/view/MockWindowComponent.h
 * DESCRIPTION: GoogleMock implementation for IWindowComponent interface.
 */

#ifndef MOCK_WINDOW_COMPONENT_H
#define MOCK_WINDOW_COMPONENT_H

#include <gmock/gmock.h>
#include "view/imguiview/interfaces/IWindowComponent.h"

namespace View {

class MockWindowComponent : public IWindowComponent {
public:
    MOCK_METHOD(void, render, (), (override));
};

} // namespace View

#endif // MOCK_WINDOW_COMPONENT_H
