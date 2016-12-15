/*
 * Copyright (C) 2010, British Broadcasting Corporation
 * All Rights Reserved.
 *
 * Author: Philip de Nier
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the British Broadcasting Corporation nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef BMX_AS02_CLIP_H_
#define BMX_AS02_CLIP_H_

#include <vector>

#include <libMXF++/MXF.h>

#include <bmx/mxf_helper/UniqueIdHelper.h>
#include <bmx/as02/AS02Track.h>



namespace bmx
{


class AS02Clip
{
public:
    friend class AS02Track;

public:
    AS02Clip(AS02Bundle *bundle, std::string filepath, mxfRational frame_rate);
    virtual ~AS02Clip();

    void SetClipName(std::string name);
    void SetStartTimecode(Timecode start_timecode);                     // default 00:00:00:00, non-drop frame
    void SetProductInfo(std::string company_name, std::string product_name, mxfProductVersion product_version,
                        std::string version, mxfUUID product_uid);
    void SetCreationDate(mxfTimestamp creation_date);                   // default generated ('now')
    void SetGenerationUID(mxfUUID generation_uid);                      // default generated
    void ReserveHeaderMetadataSpace(uint32_t min_bytes);                // default 8192

public:
    AS02Track* CreateTrack(EssenceType essence_type);

    virtual void PrepareWrite();
    void WriteSamples(uint32_t track_index, const unsigned char *data, uint32_t size, uint32_t num_samples);
    virtual void CompleteWrite();

    virtual UniqueIdHelper* GetTrackIdHelper() = 0;
    virtual UniqueIdHelper* GetStreamIdHelper() = 0;

public:
    mxfRational GetFrameRate() const { return mClipFrameRate; }

    uint32_t GetNumTracks() const { return (uint32_t)mTracks.size(); }
    int64_t GetDuration() const;

    AS02Track* GetTrack(uint32_t track_index);

    AS02Bundle* GetBundle() const { return mBundle; }

protected:
    AS02Bundle *mBundle;
    std::string mClipFilename;
    std::string mClipName;
    mxfRational mClipFrameRate;
    Timecode mStartTimecode;
    std::string mCompanyName;
    std::string mProductName;
    mxfProductVersion mProductVersion;
    std::string mVersionString;
    mxfUUID mProductUID;
    uint32_t mReserveMinBytes;
    mxfTimestamp mCreationDate;
    mxfUUID mGenerationUID;

    std::vector<AS02Track*> mTracks;
    std::map<uint32_t, AS02Track*> mTrackMap;
    uint32_t mNextVideoTrackNumber;
    uint32_t mNextAudioTrackNumber;
};


};



#endif

