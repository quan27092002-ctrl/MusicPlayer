/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/MediaFile.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of MediaFile class.
 */

#include "MediaFile.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

MediaFile::MediaFile()
    : mFilename("")
    , mPath("")
    , mDuration(0)
    , mArtist("")
    , mAlbum("") {
}

MediaFile::MediaFile(const std::string& filename, const std::string& path, 
                     uint32_t duration, const std::string& artist, const std::string& album)
    : mFilename(filename)
    , mPath(path)
    , mDuration(duration)
    , mArtist(artist)
    , mAlbum(album) {
}

// ============================================================================
// IMediaFile Interface Implementation
// ============================================================================

std::string MediaFile::getFilename() const {
    return mFilename;
}

std::string MediaFile::getPath() const {
    return mPath;
}

uint32_t MediaFile::getDuration() const {
    return mDuration;
}

std::string MediaFile::getArtist() const {
    return mArtist;
}

std::string MediaFile::getAlbum() const {
    return mAlbum;
}

bool MediaFile::isValid() const {
    return !mFilename.empty() && !mPath.empty();
}

// ============================================================================
// Setters
// ============================================================================

void MediaFile::setFilename(const std::string& filename) {
    mFilename = filename;
}

void MediaFile::setPath(const std::string& path) {
    mPath = path;
}

void MediaFile::setDuration(uint32_t duration) {
    mDuration = duration;
}

void MediaFile::setArtist(const std::string& artist) {
    mArtist = artist;
}

void MediaFile::setAlbum(const std::string& album) {
    mAlbum = album;
}

// ============================================================================
// Operators
// ============================================================================

bool MediaFile::operator==(const MediaFile& other) const {
    return mPath == other.mPath;
}

bool MediaFile::operator!=(const MediaFile& other) const {
    return !(*this == other);
}

} // namespace Model
