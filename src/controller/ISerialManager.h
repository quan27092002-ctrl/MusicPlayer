/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/ISerialManager.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Aggregate interface for serial port communication.
 *              Combines multiple small interfaces for backward compatibility.
 */

#ifndef ISERIALMANAGER_H
#define ISERIALMANAGER_H

#include "serialmanager/interfaces/ISerialConnection.h"
#include "serialmanager/interfaces/ISerialIO.h"

namespace Controller {

/**
 * @brief Aggregate interface for serial port communication.
 * 
 * Combines ISerialConnection and ISerialIO interfaces.
 * This provides backward compatibility while following ISP.
 */
class ISerialManager : public ISerialConnection,
                       public ISerialIO {
public:
    virtual ~ISerialManager() = default;
};

} // namespace Controller

#endif // ISERIALMANAGER_H
