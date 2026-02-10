/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/audioplayer/AudioVolumeImpl.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of IAudioVolume.
 */

#ifndef AUDIOVOLUME_IMPL_H
#define AUDIOVOLUME_IMPL_H

#include "interfaces/IAudioVolume.h"
#include <SDL2/SDL_mixer.h>
#include <atomic>

namespace Controller {

/**
 * @brief Concrete implementation of IAudioVolume.
 *
 * Manages audio volume control.
 */
class AudioVolumeImpl : public IAudioVolume {
public:
    AudioVolumeImpl();
    ~AudioVolumeImpl() override = default;

    // IAudioVolume interface
    void setVolume(int volume) override;
    int getVolume() const override;

private:
    std::atomic<int> mVolume;
    
    int volumeToSDL(int volume) const;
};

} // namespace Controller

#endif // AUDIOVOLUME_IMPL_H
