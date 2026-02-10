/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/playerstate/VolumeStateImpl.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Thread-safe concrete implementation of IVolumeState.
 */

#ifndef VOLUMESTATEIMPL_H
#define VOLUMESTATEIMPL_H

#include "interfaces/IVolumeState.h"
#include <atomic>

namespace Model {

/**
 * @brief Thread-safe implementation of IVolumeState.
 * 
 * Uses std::atomic for thread-safe access to volume state.
 * Follows Single Responsibility Principle (SRP).
 */
class VolumeStateImpl : public IVolumeState {
private:
    std::atomic<int> mVolume;     ///< Volume level (0-100)
    std::atomic<bool> mMuted;     ///< Mute flag

    // Volume limits
    static constexpr int MIN_VOLUME = 0;
    static constexpr int MAX_VOLUME = 100;
    static constexpr int DEFAULT_VOLUME = 50;

    // Helper to clamp volume
    int clampVolume(int volume) const;

public:
    /**
     * @brief Default constructor - initializes volume to 50, unmuted.
     */
    VolumeStateImpl();

    /**
     * @brief Parameterized constructor.
     * @param volume Initial volume level
     * @param muted Initial mute state
     */
    VolumeStateImpl(int volume, bool muted = false);

    /**
     * @brief Destructor.
     */
    ~VolumeStateImpl() override = default;

    // Delete copy (atomic members are not copyable)
    VolumeStateImpl(const VolumeStateImpl&) = delete;
    VolumeStateImpl& operator=(const VolumeStateImpl&) = delete;

    // ========================================================================
    // IVolumeState Interface Implementation
    // ========================================================================

    int getVolume() const override;
    void setVolume(int volume) override;
    bool isMuted() const override;
    void setMuted(bool muted) override;
    bool toggleMute() override;

    /**
     * @brief Reset to default state (volume 50, unmuted).
     */
    void reset();

    /**
     * @brief Get the minimum volume level.
     */
    static constexpr int getMinVolume() { return MIN_VOLUME; }

    /**
     * @brief Get the maximum volume level.
     */
    static constexpr int getMaxVolume() { return MAX_VOLUME; }
};

} // namespace Model

#endif // VOLUMESTATEIMPL_H
