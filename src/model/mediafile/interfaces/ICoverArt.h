/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/interfaces/ICoverArt.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Interface for cover art data management.
 *              Follows Interface Segregation Principle (ISP).
 */

#ifndef ICOVERART_H
#define ICOVERART_H

#include <vector>
#include <cstdint>

namespace Model {

/**
 * @brief Interface for managing cover art data.
 * 
 * This interface provides access to cover art image data.
 * It follows the Interface Segregation Principle by separating
 * cover art concerns from file info and metadata.
 */
class ICoverArt {
public:
    virtual ~ICoverArt() = default;

    /**
     * @brief Get the cover art data.
     * @return Vector containing raw image data
     */
    virtual const std::vector<uint8_t>& getCoverArt() const = 0;

    /**
     * @brief Set the cover art data.
     * @param data Raw image data
     */
    virtual void setCoverArt(const std::vector<uint8_t>& data) = 0;

    /**
     * @brief Check if cover art data exists.
     * @return true if cover art data is not empty
     */
    virtual bool hasCoverArt() const = 0;
};

} // namespace Model

#endif // ICOVERART_H
