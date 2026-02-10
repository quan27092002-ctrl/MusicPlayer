/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/mediafile/MediaMetadata.cpp
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Implementation of MediaMetadata class.
 */

#include "MediaMetadata.h"

namespace Model {

// ============================================================================
// Constructors
// ============================================================================

MediaMetadata::MediaMetadata()
    : mDuration(0)
    , mArtist("")
    , mAlbum("") {
}

MediaMetadata::MediaMetadata(uint32_t duration, 
                             const std::string& artist, 
                             const std::string& album)
    : mDuration(duration)
    , mArtist(artist)
    , mAlbum(album) {
}

// ============================================================================
// IMediaMetadata Interface Implementation
// ============================================================================

uint32_t MediaMetadata::getDuration() const {
    return mDuration;
}

void MediaMetadata::setDuration(uint32_t duration) {
    mDuration = duration;
}

std::string MediaMetadata::getArtist() const {
    return mArtist;
}

void MediaMetadata::setArtist(const std::string& artist) {
    mArtist = artist;
}

std::string MediaMetadata::getAlbum() const {
    return mAlbum;
}

void MediaMetadata::setAlbum(const std::string& album) {
    mAlbum = album;
}

} // namespace Model
