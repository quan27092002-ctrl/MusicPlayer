/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/LifecycleManager.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Manages SDL and ImGui lifecycle (SRP).
 */

#ifndef LIFECYCLEMANAGER_H
#define LIFECYCLEMANAGER_H

#include <SDL2/SDL.h>
#include <atomic>

namespace View {

class LifecycleManager {
public:
    LifecycleManager();
    ~LifecycleManager();

    bool initialize(int width, int height, const char* title);
    void shutdown();
    void processEvents(bool& running);
    
    // Getters for integration
    SDL_Window* getWindow() const { return mWindow; }
    SDL_Renderer* getRenderer() const { return mRenderer; }
    int getWindowWidth() const;
    int getWindowHeight() const;
    
    // Frame management
    void beginFrame();
    void endFrame();

private:
    SDL_Window* mWindow;
    SDL_Renderer* mRenderer;
    bool mInitialized;
    
    void setupStyle();
};

} // namespace View

#endif // LIFECYCLEMANAGER_H
