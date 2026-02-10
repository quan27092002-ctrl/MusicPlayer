/**
 * PROJECT: S32K_MediaPlayer
 * FILE: test/mocks/view/MockView.h
 * DESCRIPTION: GoogleMock implementation for IView interface.
 */

#ifndef MOCK_VIEW_H
#define MOCK_VIEW_H

#include <gmock/gmock.h>
#include "view/IView.h"

namespace View {

class MockView : public IView {
public:
    MOCK_METHOD(bool, initialize, (), (override));
    MOCK_METHOD(void, shutdown, (), (override));
    MOCK_METHOD(bool, isRunning, (), (const, override));
    MOCK_METHOD(void, processEvents, (), (override));
    MOCK_METHOD(void, render, (), (override));
};

} // namespace View

#endif // MOCK_VIEW_H
