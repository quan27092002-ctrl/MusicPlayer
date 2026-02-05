/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/controller/appcontroller/PlaylistManager.cpp
 * AUTHOR: Architecture Team
 * DESCRIPTION: Implementation of PlaylistManagerImpl.
 */

#include "PlaylistManager.h"
#include <algorithm>
#include <filesystem>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/attachedpictureframe.h>
#include <cctype>

namespace Controller {

PlaylistManagerImpl::PlaylistManagerImpl() {}

PlaylistManagerImpl::MediaFilePtr PlaylistManagerImpl::acquireMediaFile(const std::string& filePath) {
    MediaFilePtr trackPtr = findInLibrary(filePath);
    if (!trackPtr) {
        trackPtr = createMediaFile(filePath);
        if (trackPtr) {
            std::lock_guard<std::mutex> lock(mMutex);
            mMusicLibrary.push_back(trackPtr);
        }
    }
    return trackPtr;
}

void PlaylistManagerImpl::addToPlaylist(const std::string& filePath) {
    MediaFilePtr trackPtr = acquireMediaFile(filePath);
    
    // 3. Add pointer to Playlist
    if (trackPtr) {
        std::lock_guard<std::mutex> lock(mMutex);
        mPlaylist.push_back(trackPtr);
    }
}

void PlaylistManagerImpl::clearPlaylist() {
    std::lock_guard<std::mutex> lock(mMutex);
    mPlaylist.clear();
}

size_t PlaylistManagerImpl::getPlaylistSize() const {
    std::lock_guard<std::mutex> lock(mMutex);
    return mPlaylist.size();
}

size_t PlaylistManagerImpl::loadDirectory(const std::string& directoryPath) {
    size_t count = 0;
    
    try {
        for (const auto& entry : std::filesystem::directory_iterator(directoryPath)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();
                std::transform(ext.begin(), ext.end(), ext.begin(), 
                    [](unsigned char c){ return std::tolower(c); });
                
                if (ext == ".mp3" || ext == ".wav" || ext == ".ogg" || ext == ".flac") {
                    addToPlaylist(entry.path().string());
                    count++;
                }
            }
        }
    } catch (const std::exception& e) {
        (void)e;
    }
    
    return count;
}

std::string PlaylistManagerImpl::getTrackName(size_t index) const {
    auto track = getTrackAt(index);
    return track ? track->getFilename() : "";
}

std::string PlaylistManagerImpl::getTrackPath(size_t index) const {
    auto track = getTrackAt(index);
    return track ? track->getPath() : "";
}

std::string PlaylistManagerImpl::getTrackArtist(size_t index) const {
    auto track = getTrackAt(index);
    return track ? track->getArtist() : "Unknown Artist";
}

std::string PlaylistManagerImpl::getTrackAlbum(size_t index) const {
    auto track = getTrackAt(index);
    return track ? track->getAlbum() : "Unknown Album";
}

uint32_t PlaylistManagerImpl::getTrackDuration(size_t index) const {
    auto track = getTrackAt(index);
    return track ? track->getDuration() : 0;
}

std::vector<uint8_t> PlaylistManagerImpl::getTrackCoverArt(size_t index) const {
    auto track = getTrackAt(index);
    return track ? track->getCoverArt() : std::vector<uint8_t>{};
}

PlaylistManagerImpl::MediaFilePtr PlaylistManagerImpl::getTrackAt(size_t index) const {
    std::lock_guard<std::mutex> lock(mMutex);
    if (index >= mPlaylist.size()) {
        return nullptr;
    }
    auto it = mPlaylist.begin();
    std::advance(it, index);
    return *it;
}

std::list<PlaylistManagerImpl::MediaFilePtr>& PlaylistManagerImpl::getPlaylistRef() {
    return mPlaylist;
}

const std::list<PlaylistManagerImpl::MediaFilePtr>& PlaylistManagerImpl::getPlaylistRef() const {
    return mPlaylist;
}

std::vector<PlaylistManagerImpl::MediaFilePtr>& PlaylistManagerImpl::getMusicLibraryRef() {
    return mMusicLibrary;
}

std::mutex& PlaylistManagerImpl::getMutex() const {
    return mMutex;
}

PlaylistManagerImpl::MediaFilePtr PlaylistManagerImpl::findInLibrary(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(mMutex);
    for (const auto& song : mMusicLibrary) {
        if (song->getPath() == filePath) {
            return song;
        }
    }
    return nullptr;
}

PlaylistManagerImpl::MediaFilePtr PlaylistManagerImpl::createMediaFile(const std::string& filePath) {
    std::string filename = filePath;
    size_t lastSlash = filePath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        filename = filePath.substr(lastSlash + 1);
    }

    std::string artist = "Unknown Artist";
    std::string album = "Unknown Album";
    uint32_t duration = 180;
    std::vector<uint8_t> coverArt;
    
    std::string ext;
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos != std::string::npos) {
        ext = filePath.substr(dotPos);
        std::transform(ext.begin(), ext.end(), ext.begin(), 
            [](unsigned char c){ return std::tolower(c); });
    }

    TagLib::FileRef f(filePath.c_str());
    if (!f.isNull() && f.tag()) {
        TagLib::Tag *tag = f.tag();
        
        if (!tag->artist().isEmpty()) artist = tag->artist().toCString(true);
        if (!tag->album().isEmpty()) album = tag->album().toCString(true);
        
        if (f.audioProperties()) {
            duration = f.audioProperties()->lengthInSeconds();
        }

        if (ext == ".mp3") {
            TagLib::MPEG::File *mpegFile = dynamic_cast<TagLib::MPEG::File*>(f.file());
            if (mpegFile && mpegFile->ID3v2Tag()) {
                TagLib::ID3v2::Tag *id3v2 = mpegFile->ID3v2Tag();
                TagLib::ID3v2::FrameList frames = id3v2->frameListMap()["APIC"];
                
                if (!frames.isEmpty()) {
                    TagLib::ID3v2::AttachedPictureFrame *frame = 
                        dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frames.front());
                    if (frame) {
                        TagLib::ByteVector pic = frame->picture();
                        if (pic.size() > 0 && pic.size() < 5*1024*1024) {
                            coverArt.reserve(pic.size());
                            const char* data = pic.data();
                            coverArt.assign(data, data + pic.size());
                        }
                    }
                }
            }
        }
    }
    
    if (duration == 0) duration = 180;

    return std::make_shared<Model::MediaFile>(filename, filePath, duration, artist, album, coverArt);
}

} // namespace Controller
