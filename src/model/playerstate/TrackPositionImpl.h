/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/TrackPositionImpl.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Thread-safe concrete implementation of ITrackPosition.
 */

#ifndef TRACKPOSITIONIMPL_H
#define TRACKPOSITIONIMPL_H

#include "interfaces/ITrackPosition.h"
#include <atomic>
#include <cstdint>

namespace Model {

/**
 * @brief Thread-safe implementation of ITrackPosition.
 * 
 * Uses std::atomic for thread-safe access to track position.
 * Follows Single Responsibility Principle (SRP).
 */
class TrackPositionImpl : public ITrackPosition {
private:
    std::atomic<uint32_t> mPosition;  ///< Current position in seconds
    std::atomic<uint32_t> mPlaybackVersion;  ///< Increments on repeat restart

public:
    /**
     * @brief Default constructor - initializes to position 0.
     */
    TrackPositionImpl();

    /**
     * @brief Parameterized constructor.
     * @param position Initial position in seconds
     */
    explicit TrackPositionImpl(uint32_t position);

    /**
     * @brief Destructor.
     */
    ~TrackPositionImpl() override = default;

    // Delete copy (atomic members are not copyable)
    TrackPositionImpl(const TrackPositionImpl&) = delete;
    TrackPositionImpl& operator=(const TrackPositionImpl&) = delete;

    // ========================================================================
    // ITrackPosition Interface Implementation
    // ========================================================================

    uint32_t getCurrentPosition() const override;
    void setCurrentPosition(uint32_t position) override;
    uint32_t getPlaybackVersion() const override;
    void incrementPlaybackVersion() override;

    /**
     * @brief Reset to default state (position 0).
     */
    void reset();
};

} // namespace Model

#endif // TRACKPOSITIONIMPL_H
