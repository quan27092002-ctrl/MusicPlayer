/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/IAudioPlayer.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Aggregate interface for audio playback functionality.
 *              Combines multiple small interfaces for backward compatibility.
 */

#ifndef IAUDIOPLAYER_H
#define IAUDIOPLAYER_H

#include "audioplayer/interfaces/IAudioLifecycle.h"
#include "audioplayer/interfaces/IAudioLoader.h"
#include "audioplayer/interfaces/IAudioPlayback.h"
#include "audioplayer/interfaces/IAudioVolume.h"

namespace Controller {

/**
 * @brief Aggregate interface for audio player.
 * 
 * Combines IAudioLifecycle, IAudioLoader, IAudioPlayback, and IAudioVolume.
 * This provides backward compatibility while following ISP.
 */
class IAudioPlayer : public IAudioLifecycle,
                     public IAudioLoader,
                     public IAudioPlayback,
                     public IAudioVolume {
public:
    virtual ~IAudioPlayer() = default;
};

} // namespace Controller

#endif // IAUDIOPLAYER_H
