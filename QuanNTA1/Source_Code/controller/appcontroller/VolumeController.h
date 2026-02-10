/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/VolumeController.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Concrete implementation of IVolumeController.
 *              Follows Single Responsibility Principle (SRP).
 */

#ifndef VOLUMECONTROLLER_IMPL_H
#define VOLUMECONTROLLER_IMPL_H

#include "interfaces/IVolumeController.h"
#include "../IAudioPlayer.h"
#include "../../model/IPlayerState.h"
#include <memory>

namespace Controller {

/**
 * @brief Concrete implementation of IVolumeController.
 * 
 * Manages volume level and mute state.
 */
class VolumeControllerImpl : public IVolumeController {
public:
    VolumeControllerImpl(
        std::shared_ptr<IAudioPlayer> audioPlayer,
        std::shared_ptr<Model::IPlayerState> playerState
    );
    ~VolumeControllerImpl() override = default;
    
    // IVolumeController interface
    void setVolume(int volume) override;
    int getVolume() const override;
    void toggleMute() override;
    
private:
    std::shared_ptr<IAudioPlayer> mAudioPlayer;
    std::shared_ptr<Model::IPlayerState> mPlayerState;
    int mVolumeBeforeMute;
};

} // namespace Controller

#endif // VOLUMECONTROLLER_IMPL_H
