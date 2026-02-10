/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/CoverArt.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of CoverArt class.
 */

#include "CoverArt.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

CoverArt::CoverArt()
    : mData() {
}

CoverArt::CoverArt(const std::vector<uint8_t>& data)
    : mData(data) {
}

// ============================================================================
// ICoverArt Interface Implementation
// ============================================================================

const std::vector<uint8_t>& CoverArt::getCoverArt() const {
    return mData;
}

void CoverArt::setCoverArt(const std::vector<uint8_t>& data) {
    mData = data;
}

bool CoverArt::hasCoverArt() const {
    return !mData.empty();
}

// ============================================================================
// Additional Methods
// ============================================================================

void CoverArt::clear() {
    mData.clear();
}

size_t CoverArt::size() const {
    return mData.size();
}

} // namespace Model
