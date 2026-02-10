/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/main.cpp
 * AUTHOR: QuanNTA1
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
    // === S32K Media Player Initialization ===


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

    // Auto-load music directory (async for faster startup)
    std::cout << "Loading music from ./mMusic/ ..." << std::endl;
    appController->setLoadProgressCallback([](size_t loaded, size_t total) {
        std::cout << "Loaded " << loaded << "/" << total << " tracks." << std::endl;
    });
    appController->loadDirectoryAsync("./mMusic", 50);
    
    // Note: First batch (50 songs) loads sync, so UI has content immediately
    // Remaining songs load in background

    std::cout << "Initialization complete. Running..." << std::endl;

    // Main loop
    while (view->isRunning()) {
        view->processEvents();
        view->render();
    }

    // Cleanup - shutdown audio first, then SDL/View
    std::cout << "Shutting down..." << std::endl;
    appController->shutdown();  // Shutdown audio first
    view->shutdown();           // Then SDL/ImGui

    std::cout << "Goodbye!" << std::endl;
    return 0;
}
