/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/CoverArt.h
 * AUTHOR: Architecture Team
 * DESCRIPTION: Concrete implementation of ICoverArt.
 */

#ifndef COVERART_H
#define COVERART_H

#include "interfaces/ICoverArt.h"
#include <vector>
#include <cstddef>
#include <cstdint>

namespace Model {

/**
 * @brief Concrete implementation of ICoverArt.
 * 
 * Stores cover art image data.
 * Follows Single Responsibility Principle (SRP).
 */
class CoverArt : public ICoverArt {
private:
    std::vector<uint8_t> mData;  ///< Raw image data from ID3 tag

public:
    /**
     * @brief Default constructor - creates empty cover art.
     */
    CoverArt();

    /**
     * @brief Parameterized constructor.
     * @param data Raw image data
     */
    explicit CoverArt(const std::vector<uint8_t>& data);

    /**
     * @brief Copy constructor.
     */
    CoverArt(const CoverArt& other) = default;

    /**
     * @brief Move constructor.
     */
    CoverArt(CoverArt&& other) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    CoverArt& operator=(const CoverArt& other) = default;

    /**
     * @brief Move assignment operator.
     */
    CoverArt& operator=(CoverArt&& other) noexcept = default;

    /**
     * @brief Destructor.
     */
    ~CoverArt() override = default;

    // ========================================================================
    // ICoverArt Interface Implementation
    // ========================================================================

    const std::vector<uint8_t>& getCoverArt() const override;
    void setCoverArt(const std::vector<uint8_t>& data) override;
    bool hasCoverArt() const override;

    /**
     * @brief Clear the cover art data.
     */
    void clear();

    /**
     * @brief Get the size of cover art data in bytes.
     * @return Size in bytes
     */
    size_t size() const;
};

} // namespace Model

#endif // COVERART_H
