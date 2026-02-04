/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/MediaFileInfo.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of MediaFileInfo class.
 */

#include "MediaFileInfo.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

MediaFileInfo::MediaFileInfo()
    : mFilename("")
    , mPath("") {
}

MediaFileInfo::MediaFileInfo(const std::string& filename, const std::string& path)
    : mFilename(filename)
    , mPath(path) {
}

// ============================================================================
// IMediaFileInfo Interface Implementation
// ============================================================================

std::string MediaFileInfo::getFilename() const {
    return mFilename;
}

void MediaFileInfo::setFilename(const std::string& filename) {
    mFilename = filename;
}

std::string MediaFileInfo::getPath() const {
    return mPath;
}

void MediaFileInfo::setPath(const std::string& path) {
    mPath = path;
}

bool MediaFileInfo::isValid() const {
    return !mFilename.empty() && !mPath.empty();
}

// ============================================================================
// Operators
// ============================================================================

bool MediaFileInfo::operator==(const MediaFileInfo& other) const {
    return mPath == other.mPath;
}

bool MediaFileInfo::operator!=(const MediaFileInfo& other) const {
    return !(*this == other);
}

} // namespace Model
