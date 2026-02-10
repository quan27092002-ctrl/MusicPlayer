/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/view/imguiview/interfaces/IWindowComponent.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Interface for UI components.
 */

#ifndef IWINDOWCOMPONENT_H
#define IWINDOWCOMPONENT_H

namespace View {

/**
 * @brief Interface for renderable UI components.
 * 
 * Adheres to ISP by providing a focused interface for rendering.
 */
class IWindowComponent {
public:
    virtual ~IWindowComponent() = default;

    /**
     * @brief Render the component.
     * Should be called within an ImGui frame.
     */
    virtual void render() = 0;
};

} // namespace View

#endif // IWINDOWCOMPONENT_H
