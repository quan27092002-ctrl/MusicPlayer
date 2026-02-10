/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/MediaFile.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of MediaFile class using composition.
 */

#include "MediaFile.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

MediaFile::MediaFile()
    : mFileInfo()
    , mMetadata()
    , mCoverArt() {
}

MediaFile::MediaFile(const std::string& filename, const std::string& path, 
                     uint32_t duration, const std::string& artist, 
                     const std::string& album, const std::vector<uint8_t>& coverArt)
    : mFileInfo(filename, path)
    , mMetadata(duration, artist, album)
    , mCoverArt(coverArt) {
}

// ============================================================================
// IMediaFileInfo Interface Implementation (delegated)
// ============================================================================

std::string MediaFile::getFilename() const {
    return mFileInfo.getFilename();
}

void MediaFile::setFilename(const std::string& filename) {
    mFileInfo.setFilename(filename);
}

std::string MediaFile::getPath() const {
    return mFileInfo.getPath();
}

void MediaFile::setPath(const std::string& path) {
    mFileInfo.setPath(path);
}

bool MediaFile::isValid() const {
    return mFileInfo.isValid();
}

// ============================================================================
// IMediaMetadata Interface Implementation (delegated)
// ============================================================================

uint32_t MediaFile::getDuration() const {
    return mMetadata.getDuration();
}

void MediaFile::setDuration(uint32_t duration) {
    mMetadata.setDuration(duration);
}

std::string MediaFile::getArtist() const {
    return mMetadata.getArtist();
}

void MediaFile::setArtist(const std::string& artist) {
    mMetadata.setArtist(artist);
}

std::string MediaFile::getAlbum() const {
    return mMetadata.getAlbum();
}

void MediaFile::setAlbum(const std::string& album) {
    mMetadata.setAlbum(album);
}

// ============================================================================
// ICoverArt Interface Implementation (delegated)
// ============================================================================

const std::vector<uint8_t>& MediaFile::getCoverArt() const {
    return mCoverArt.getCoverArt();
}

void MediaFile::setCoverArt(const std::vector<uint8_t>& data) {
    mCoverArt.setCoverArt(data);
}

bool MediaFile::hasCoverArt() const {
    return mCoverArt.hasCoverArt();
}

// ============================================================================
// Operators
// ============================================================================

bool MediaFile::operator==(const MediaFile& other) const {
    return mFileInfo.getPath() == other.mFileInfo.getPath();
}

bool MediaFile::operator!=(const MediaFile& other) const {
    return !(*this == other);
}

} // namespace Model
