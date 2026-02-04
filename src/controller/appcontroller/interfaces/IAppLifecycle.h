/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/interfaces/IAppLifecycle.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for application lifecycle management.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef IAPPLIFECYCLE_H
#define IAPPLIFECYCLE_H

#include <functional>

namespace Controller {

/**
 * @brief Application state enumeration.
 */
enum class AppState {
    UNINITIALIZED = 0,  ///< Not yet initialized
    READY = 1,          ///< Initialized and ready
    RUNNING = 2,        ///< Running (connected to board)
    ERROR = 3           ///< Error state
};

/**
 * @brief Callback for application state changes.
 */
using AppStateCallback = std::function<void(AppState state)>;

/**
 * @brief Interface for application lifecycle management.
 * 
 * This interface provides access to initialization, shutdown,
 * and state management operations.
 */
class IAppLifecycle {
public:
    virtual ~IAppLifecycle() = default;

    /**
     * @brief Initialize all subsystems.
     * @return true if all subsystems initialized successfully
     */
    virtual bool initialize() = 0;

    /**
     * @brief Shutdown all subsystems.
     */
    virtual void shutdown() = 0;

    /**
     * @brief Get current application state.
     * @return Current AppState
     */
    virtual AppState getState() const = 0;

    /**
     * @brief Set callback for application state changes.
     * @param callback Function to call on state change
     */
    virtual void setStateCallback(AppStateCallback callback) = 0;
};

} // namespace Controller

#endif // IAPPLIFECYCLE_H
