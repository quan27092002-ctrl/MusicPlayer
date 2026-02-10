/**
 * PROJECT: S32K_MediaPlayer
 * FILE: src/model/IMediaFile.h
 * AUTHOR: QuanNTA1
 * DESCRIPTION: Aggregate interface for MediaFile - combines IMediaFileInfo, 
 *              IMediaMetadata, and ICoverArt interfaces.
 *              Provides backward compatibility while following ISP.
 */

#ifndef IMEDIAFILE_H
#define IMEDIAFILE_H

#include "mediafile/interfaces/IMediaFileInfo.h"
#include "mediafile/interfaces/IMediaMetadata.h"
#include "mediafile/interfaces/ICoverArt.h"

namespace Model {

/**
 * @brief Aggregate interface for a media file.
 * 
 * This interface combines IMediaFileInfo, IMediaMetadata, and ICoverArt
 * into a single interface for backward compatibility. It allows clients
 * that need all media file features to depend on this single interface,
 * while clients that only need specific features can depend on the
 * smaller interfaces directly (Interface Segregation Principle).
 */
class IMediaFile : public IMediaFileInfo, 
                   public IMediaMetadata, 
                   public ICoverArt {
public:
    virtual ~IMediaFile() = default;
};

} // namespace Model

#endif // IMEDIAFILE_H
