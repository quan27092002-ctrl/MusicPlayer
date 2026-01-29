/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/IView.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for View layer in MVC pattern.
 */

#ifndef IVIEW_H
#define IVIEW_H

#include <string>
#include <vector>

namespace View {

/**
 * @brief Abstract interface for the View layer.
 * 
 * Defines the contract for UI implementations.
 * Could be ConsoleView, ImGuiView, QtView, etc.
 */
class IView {
public:
    virtual ~IView() = default;

    // ========================================================================
    // Lifecycle
    // ========================================================================

    /**
     * @brief Initialize the view (create window, etc.)
     * @return true if initialization successful
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown and cleanup resources.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Check if view is still running.
     * @return true if view should continue running
     */
    virtual bool isRunning() const = 0;

    // ========================================================================
    // Main Loop
    // ========================================================================

    /**
     * @brief Process input events (keyboard, mouse, window).
     * Call this at the start of each frame.
     */
    virtual void processEvents() = 0;

    /**
     * @brief Render the current frame.
     * Reads state from PlayerState and displays UI.
     */
    virtual void render() = 0;
};

} // namespace View

#endif // IVIEW_H
