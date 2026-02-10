/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/interfaces/ILifecycleManager.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for SDL/ImGui lifecycle management.
 */

#ifndef ILIFECYCLEMANAGER_H
#define ILIFECYCLEMANAGER_H

#include <string>

struct SDL_Window;
struct SDL_Renderer;

namespace View {

/**
 * @brief Interface for managing SDL window and ImGui lifecycle.
 */
class ILifecycleManager {
public:
    virtual ~ILifecycleManager() = default;

    virtual bool initialize(int width, int height, const std::string& title) = 0;
    virtual void shutdown() = 0;
    virtual void processEvents(bool& running) = 0;
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    
    virtual SDL_Window* getWindow() const = 0;
    virtual SDL_Renderer* getRenderer() const = 0;
    virtual int getWindowWidth() const = 0;
    virtual int getWindowHeight() const = 0;
};

} // namespace View

#endif // ILIFECYCLEMANAGER_H
