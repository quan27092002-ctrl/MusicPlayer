/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/main.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Application entry point.
 */

#include <memory>
#include <iostream>

#include "model/PlayerState.h"
#include "controller/AudioPlayer.h"
#include "controller/SerialManager.h"
#include "controller/AppController.h"
#include "view/ImGuiView.h"

int main(int /*argc*/, char** /*argv*/) {
    std::cout << "=== S32K Media Player ===" << std::endl;

    // Create shared components
    auto playerState = std::make_shared<Model::PlayerState>();
    auto audioPlayer = std::make_shared<Controller::AudioPlayer>();
    auto serialManager = std::make_shared<Controller::SerialManager>();
    auto appController = std::make_shared<Controller::AppController>(
        audioPlayer, serialManager, playerState
    );

    // Create view with dependencies
    auto view = std::make_unique<View::ImGuiView>(appController, playerState);

    // Initialize controller
    if (!appController->initialize()) {
        std::cerr << "Failed to initialize AppController!" << std::endl;
        return 1;
    }

    // Initialize view
    if (!view->initialize()) {
        std::cerr << "Failed to initialize View!" << std::endl;
        appController->shutdown();
        return 1;
    }

    std::cout << "Initialization complete. Running..." << std::endl;

    // Main loop
    while (view->isRunning()) {
        view->processEvents();
        view->render();
    }

    // Cleanup
    std::cout << "Shutting down..." << std::endl;
    view->shutdown();
    appController->shutdown();

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
